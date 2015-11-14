#include <network/graph_writer.h>
#include <solver/bc/callbacks/cuts_callback.h>
#include <solver/bc/callbacks/cuts_lazy_constraint.h>
#include <solver/bc/callbacks/print_relaxation_graph_callback.h>
#include <solver/bc/bc_solver.h>

#include <ilcplex/ilocplex.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ratio>
#include <sstream>
#include <stdexcept>

bc_solver::bc_solver(tsp_graph& g, const program_params& params, program_data& data, const std::vector<path>& initial_solutions) : g{g}, params{params}, data{data}, initial_solutions{initial_solutions} {
    add_initial_solution_vals();
    create_results_dir(0750, params.bc.results_dir + g.g[graph_bundle].instance_dir);
}

void bc_solver::add_initial_solution_vals() {
    auto n = g.g[graph_bundle].n;
    
    initial_x_val = std::vector<values_matrix>(initial_solutions.size());
    initial_y_val = std::vector<values_matrix>(initial_solutions.size());
    
    for(auto sol_n = 0u; sol_n < initial_solutions.size(); sol_n++) {
        if(!initial_solutions.at(sol_n).verify_feasible(g)) {
            std::cerr << "bc_solver.cpp::add_initial_solution_vals() \t I have an unfeasible initial solution!" << std::endl;
        }
        
        auto initial_x_sol = values_matrix(2 * n + 2, std::vector<int>(2 * n + 2, 0));
        auto initial_y_sol = values_matrix(2 * n + 2, std::vector<int>(2 * n + 2, 0));

        for(auto l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) {
                initial_x_sol[initial_solutions.at(sol_n).path_v[l]][initial_solutions.at(sol_n).path_v[l+1]] = 1;
                initial_y_sol[initial_solutions.at(sol_n).path_v[l]][initial_solutions.at(sol_n).path_v[l+1]] = initial_solutions.at(sol_n).load_v[l];
            }
        }
    
        initial_x_val[sol_n] = initial_x_sol;
        initial_y_val[sol_n] = initial_y_sol;
    }
}

path bc_solver::solve_for_k_opt(const path& solution, const std::vector<std::vector<int>>& lhs, int rhs) {
    k_opt_lhs = lhs;
    k_opt_rhs = rhs;
    
    initial_solutions.clear();
    initial_solutions.push_back(solution);
    
    add_initial_solution_vals();
    
    return solve(true);
}

void bc_solver::solve_with_branch_and_cut() {
    solve(false);
}

path bc_solver::solve(bool k_opt) {
    using namespace std::chrono;

    auto unfeasible_paths_n = std::count_if(g.infeas_list.begin(), g.infeas_list.end(),
        [] (const auto& kv) -> bool {
            return kv.second;
        }
    );

    if(DEBUG) {
        std::cerr << "bc_solver.cpp::solve() \t Invoked with k_opt = " << std::boolalpha << k_opt;
        if(!initial_solutions.empty()) {
            std::cerr << " (initial solution value: " << initial_solutions.back().total_cost << ")" << std::endl;
        } else {
            std::cerr << std::endl;
        }
    
        std::cerr << "bc_solver.cpp::solve() \t Currently have " << unfeasible_paths_n << " precomputed unfeasible sub-paths" << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t Adding up to " << params.bc.max_infeas_subpaths << " unfeasible sub-paths to the model" << std::endl;
    }

    auto total_bb_nodes_explored = (long)0;
    auto number_of_cuts_added_at_root = (long)0;
    auto time_spent_at_root = 0.0;
    auto ub_at_root = 0.0;
    auto lb_at_root = 0.0;
    auto ub = 0.0;
    auto lb = 0.0;
    auto total_cplex_time = 0.0;
    
    data.reset_for_new_branch_and_cut();

    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;

    IloEnv env;
    IloModel model(env);

    IloNumVarArray variables_x(env);
    IloNumVarArray variables_y(env);
    
    IloRangeArray outdegree(env);
    IloRangeArray indegree(env);
    IloRangeArray y_upper(env);
    IloRangeArray y_lower(env);
    IloRangeArray load(env);
    IloRange      initial_load(env, 0.0, 0.0, "initial_load");
    IloRangeArray two_cycles_elimination(env);
    IloRangeArray subpath_elimination(env);
    IloRangeArray clique(env);
    IloRange      k_opt_constraint(env, k_opt_rhs, IloInfinity, "k_opt_constraint");

    IloObjective obj = IloMinimize(env);
    
    auto t_model_start = high_resolution_clock::now();

    #include <solver/bc/bc_setup_model.raw.cpp>
    
    auto t_model_end = high_resolution_clock::now();
    auto model_time_span = duration_cast<duration<double>>(t_model_end - t_model_start);
    
    if(DEBUG) {
        std::cerr << "bc_solver.cpp::solve() \t Model built in " << model_time_span.count() << " seconds" << std::endl;
    }
    
    model.add(obj);
    model.add(variables_x);
    model.add(variables_y);
    
    model.add(outdegree);
    model.add(indegree);
    model.add(y_upper);
    model.add(y_lower);
    model.add(load);
    model.add(initial_load);
    if(k_opt || params.bc.two_cycles_elim) {
        model.add(two_cycles_elimination);
    }
    if(k_opt || params.bc.subpath_elim) {
        model.add(subpath_elimination);
    }
    if(k_opt) {
        model.add(k_opt_constraint);
    }
    
    IloCplex cplex(model);
     
    // Add initial solutions
    for(auto sol_n = 0u; sol_n < initial_solutions.size(); sol_n++) {
        IloNumVarArray initial_vars(env);
        IloNumArray initial_values(env);

        auto col_idx = 0;
        for(auto i = 0; i <= 2 * n + 1; i++) {
            for(auto j = 0; j <= 2 * n + 1; j++) {
                if(g.cost[i][j] >= 0) {
                    initial_vars.add(variables_x[col_idx]);
                    initial_values.add(initial_x_val[sol_n][i][j]);
                    initial_vars.add(variables_y[col_idx]);
                    initial_values.add(initial_y_val[sol_n][i][j]);
                    col_idx++;
                }
            }
        }

        cplex.addMIPStart(initial_vars, initial_values);

        IloConstraintArray constraints(env);
        constraints.add(outdegree); constraints.add(indegree);
        constraints.add(y_upper); constraints.add(y_lower);
        constraints.add(load); constraints.add(initial_load);
        if(k_opt || params.bc.two_cycles_elim) { constraints.add(two_cycles_elimination); }
        if(k_opt) { constraints.add(k_opt_constraint); }

        IloNumArray preferences(env);
        for(auto i = 0; i < constraints.getSize(); i++) { preferences.add(1.0); }

        if(cplex.refineMIPStartConflict(0, constraints, preferences)) {
            IloCplex::ConflictStatusArray conflict = cplex.getConflict(constraints);
            env.getImpl()->useDetailedDisplay(IloTrue);
            for(auto i = 0; i < constraints.getSize(); i++) {
                if(conflict[i] == IloCplex::ConflictMember) {
                    std::cerr << "bc_solver.cpp::solve() \t Proven conflict: " << constraints[i] << std::endl;
                }
                if(conflict[i] == IloCplex::ConflictPossibleMember) {
                    std::cerr << "bc_solver.cpp::solve() \t Possible conflict: " << constraints[i] << std::endl;
                }
            }
        }

        initial_values.end();
        initial_vars.end();
    }

    // Add callbacks to separate cuts
    auto gr_with_reverse = g.make_reverse_tsp_graph();
    auto last_solution = IloNumArray(env);
    cplex.use(cuts_lazy_constraint_handle(env, variables_x, g, gr_with_reverse, data));
    cplex.use(cuts_callback_handle(env, variables_x, k_opt, g, gr_with_reverse, params, data, last_solution));
    
    // Add callback to print graphviz stuff
    if(!k_opt && params.bc.print_relaxation_graph) {
        cplex.use(print_relaxation_graph_callback_handle(env, variables_x, variables_y, g));
    }

    // Export model to file
    if(!k_opt) {
        cplex.exportModel("model.lp");
    }
    
    // Set CPLEX parameters
    cplex.setParam(IloCplex::TiLim, params.cplex_timeout);
    cplex.setParam(IloCplex::Threads, params.cplex_threads);
    cplex.setParam(IloCplex::Param::Parallel, IloCplex::Opportunistic);
    cplex.setParam(IloCplex::NodeLim, 0);
    
    if(k_opt && !DEBUG) {
        cplex.setOut(env.getNullStream());
    }
    
    auto t_start = high_resolution_clock::now();

    // Solve root node
    auto success_at_root_node = cplex.solve();

    if(!success_at_root_node && cplex.getCplexStatus() != IloCplex::NodeLimInfeas) {
        std::cerr << "bc_solver.cpp::solve() \t CPLEX problem encountered at root node" << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t CPLEX status: " << cplex.getStatus() << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t CPLEX ext status: " << cplex.getCplexStatus() << std::endl;
        
        cplex.exportModel("model_err.lp");
        throw std::runtime_error("Some error occurred or the problem is infeasible");
    }
    
    auto t_end = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t_end - t_start);
    
    time_spent_at_root = time_span.count();
    number_of_cuts_added_at_root =
        data.total_number_of_feasibility_cuts_added +
        data.total_number_of_subtour_elimination_vi_added +
        data.total_number_of_generalised_order_vi_added +
        data.total_number_of_capacity_vi_added + 
        data.total_number_of_simplified_fork_vi_added + 
        data.total_number_of_fork_vi_added +
        data.total_number_of_infork_vi_added + 
        data.total_number_of_outfork_vi_added;
    lb_at_root = cplex.getBestObjValue();
    
    if(cplex.isPrimalFeasible()) {
        ub_at_root = cplex.getObjValue();
    } else {
        ub_at_root = std::numeric_limits<double>::max();
    }

    // Solve the rest of the BB tree
    cplex.setParam(IloCplex::NodeLim, 2100000000);
    
    cplex.solve();
    
    auto t_end_total = high_resolution_clock::now();
    auto time_span_total = duration_cast<duration<double>>(t_end_total - t_start);

    if(!k_opt) {
        std::cerr << "bc_solver.cpp::solve() \t CPLEX status: " << cplex.getStatus() << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t CPLEX ext status: " << cplex.getCplexStatus() << std::endl;
        if(cplex.isPrimalFeasible()) {
            std::cerr << "bc_solver.cpp::solve() \t Objective value: " << cplex.getObjValue() << std::endl;
        }
    }

    total_bb_nodes_explored = cplex.getNnodes();
    lb = cplex.getBestObjValue();
    
    if(cplex.isPrimalFeasible()) {
        ub = cplex.getObjValue();
    } else {
        ub = std::numeric_limits<double>::max();
    }
    
    total_cplex_time = time_span_total.count();
    
    if(k_opt) { data.time_spent_by_k_opt_heuristics += total_cplex_time; }
    
    auto opt_solution_path = path();
    
    if(cplex.isPrimalFeasible()) {
        // Get solution
        IloNumArray x(env);
        auto col_index = 0;
        auto solution_x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
        
        cplex.getValues(x, variables_x);
        
        for(auto i = 0; i <= 2 * n + 1; i++) {
            for(auto j = 0; j <= 2 * n + 1; j++) {
                if(g.cost[i][j] >= 0) {
                    if(x[col_index++] > eps) {
                        solution_x[i][j] = 1;
                    }
                }
            }
        }
        
        x.end();
        
        if(DEBUG && !k_opt) {
            print_x_variables(solution_x);
        }
        
        opt_solution_path = path(g, solution_x);
    
        if(!opt_solution_path.verify_feasible(g)) {
            std::cerr << "bc_solver.cpp::solve() \t The optimal solution is infeasible!" << std::endl;
        }
    }
    
    if(!k_opt) {
        print_results(total_cplex_time, time_spent_at_root, ub, lb, ub_at_root, lb_at_root, number_of_cuts_added_at_root, unfeasible_paths_n, total_bb_nodes_explored);
    }

    env.end();

    return opt_solution_path;
}

void bc_solver::print_x_variables(std::vector<std::vector<int>> x) {
    auto n = g.g[graph_bundle].n;
    
    std::ofstream x_vars_file;
    x_vars_file.open("x_vars.txt", std::ios::out);
    
    for(auto i = 0; i <= 2 * n + 1; i++) {
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(g.cost[i][j] >= 0) {
                x_vars_file << "x_" << i << "_" << j << " = " << x[i][j] << "; ";
            }
        }
    }
    
    x_vars_file << std::endl;
    x_vars_file.close();
}

void bc_solver::print_results(double total_cplex_time, double time_spent_at_root, double ub, double lb, double ub_at_root, double lb_at_root, double number_of_cuts_added_at_root, double unfeasible_paths_n, double total_bb_nodes_explored) {
    std::ofstream results_file;
    results_file.open(params.bc.results_dir + g.g[graph_bundle].instance_dir + "/results.txt", std::ios::out | std::ios::app);

    results_file << g.g[graph_bundle].instance_name << "\t";
    results_file << g.g[graph_bundle].n << "\t";
    results_file << g.g[graph_bundle].h << "\t";
    results_file << g.g[graph_bundle].k << "\t";

    // TIMES
    results_file << total_cplex_time << "\t";
    results_file << time_spent_at_root << "\t";
    results_file << data.time_spent_by_constructive_heuristics << "\t";
    results_file << data.time_spent_by_k_opt_heuristics << "\t";
    results_file << data.time_spent_by_tabu_search << "\t";
    results_file << data.time_spent_separating_feasibility_cuts << "\t";
    if(params.bc.subtour_elim.enabled) {
        results_file << data.time_spent_separating_subtour_elimination_vi << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.generalised_order.enabled) {
        results_file << data.time_spent_separating_generalised_order_vi << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.capacity.enabled) {
        results_file << data.time_spent_separating_capacity_vi << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.fork.enabled) {
        results_file << data.time_spent_separating_fork_vi << "\t";
    } else {
        results_file << "no\t";
    }

    // SOLUTIONS
    results_file << data.n_constructive_solutions << "\t";
    results_file << data.best_constructive_solution << "\t";
    results_file << data.best_tabu_solution << "\t";
    results_file << ub << "\t";
    results_file << lb << "\t";
    results_file << (ub - lb) / ub << "\t";
    results_file << ub_at_root << "\t";
    results_file << lb_at_root << "\t";
    results_file << (ub_at_root - lb_at_root) / ub_at_root << "\t";

    // CUTS ADDED
    results_file << data.total_number_of_feasibility_cuts_added << "\t";
    if(params.bc.subtour_elim.enabled) {
        results_file << data.total_number_of_subtour_elimination_vi_added << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.generalised_order.enabled) {
        results_file << data.total_number_of_generalised_order_vi_added << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.capacity.enabled) {
        results_file << data.total_number_of_capacity_vi_added << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.fork.enabled) {
        results_file << data.total_number_of_fork_vi_added << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.fork.lifted) {
        results_file << data.total_number_of_outfork_vi_added << "\t";
        results_file << data.total_number_of_infork_vi_added << "\t";
    } else {
        results_file << "no\tno\t";
    }
    results_file << number_of_cuts_added_at_root << "\t";

    // CUTS SEPARATED EVERY N NODES
    if(params.bc.subtour_elim.enabled) {
        results_file << params.bc.subtour_elim.n1 << ",";
        results_file << params.bc.subtour_elim.n2 << ",";
        results_file << params.bc.subtour_elim.p1 << ",";
        results_file << params.bc.subtour_elim.p2 << ",";
        results_file << params.bc.subtour_elim.p3 << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.generalised_order.enabled) {
        results_file << params.bc.generalised_order.n1 << ",";
        results_file << params.bc.generalised_order.n2 << ",";
        results_file << params.bc.generalised_order.p1 << ",";
        results_file << params.bc.generalised_order.p2 << ",";
        results_file << params.bc.generalised_order.p3 << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.capacity.enabled) {
        results_file << params.bc.capacity.n1 << ",";
        results_file << params.bc.capacity.n2 << ",";
        results_file << params.bc.capacity.p1 << ",";
        results_file << params.bc.capacity.p2 << ",";
        results_file << params.bc.capacity.p3 << "\t";
    } else {
        results_file << "no\t";
    }
    if(params.bc.fork.enabled) {
        results_file << params.bc.fork.n1 << ",";
        results_file << params.bc.fork.n2 << ",";
        results_file << params.bc.fork.p1 << ",";
        results_file << params.bc.fork.p2 << ",";
        results_file << params.bc.fork.p3 << "\t";
    } else {
        results_file << "no\t";
    }

    // 2-CYCLE ELIMINATION
    if(params.bc.two_cycles_elim) {
        results_file << "yes\t";
    } else {
        results_file << "no\t";
    }

    // UNFEASIBLE PATHS
    if(params.bc.subpath_elim) {
        results_file << unfeasible_paths_n << "\t";
    } else {
        results_file << "no\t";
    }

    // BB NODES
    results_file << total_bb_nodes_explored << "\t";
    
    // ARCS IN GRAPH
    results_file << num_edges(g.g) << std::endl;

    results_file.close();
}

// NON-PORTABLE
void bc_solver::create_results_dir(mode_t mode, const std::string& dir) {
    struct stat st;
    auto iter = dir.begin();

    while(iter != dir.end()) {
        auto new_iter = std::find(iter, dir.end(), '/');
        auto new_dir = "./" + std::string(dir.begin(), new_iter);

        if(stat(new_dir.c_str(), &st) != 0) {            
            if(mkdir(new_dir.c_str(), mode) != 0 && errno != EEXIST) {
                std::cerr << "bc_solver.cpp::create_results_dir() \t Cannot create folder " << new_dir << ": " << strerror(errno) << std::endl;
                throw std::runtime_error("Cannot create results folder");
            }
        } else {
            if(!S_ISDIR(st.st_mode)) {
                errno = ENOTDIR;
                std::cerr << "bc_solver.cpp::create_results_dir() \t Path " << new_dir << " is not a directory" << std::endl;
                throw std::runtime_error("Cannot create results folder");
            }
        }

        iter = new_iter;
        if(new_iter != dir.end()) {
            ++iter;
        }
    }
}