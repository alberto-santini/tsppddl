#include <global.h>
#include <network/graph_writer.h>
#include <solver/bc/callbacks/cuts_callback.h>
#include <solver/bc/callbacks/cuts_lazy_constraint.h>
#include <solver/bc/callbacks/print_relaxation_graph_callback.h>
#include <solver/bc/bc_solver.h>

#include <ilcplex/ilocplex.h>

// #include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ratio>
#include <sstream>
#include <stdexcept>

bc_solver::bc_solver(tsp_graph& g, const program_params& params, const std::vector<path>& initial_solutions, const std::string& instance_path) : g{g}, params{params}, initial_solutions{initial_solutions} {
    add_initial_solution_vals();
    
    // PORTABLE WAY:
    // boost::filesystem::path i_path(instance_path);
    // std::stringstream ss; ss << i_path.stem();
    // instance_name = ss.str();
    
    // NOT-PORTABLE WAY:
    auto path_parts = std::vector<std::string>();
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    auto file_parts = std::vector<std::string>();
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));
    if(file_parts.size() > 1) {
        file_parts.pop_back();
    }
    
    instance_name = boost::algorithm::join(file_parts, ".");
}

void bc_solver::add_initial_solution_vals() {
    auto n = g.g[graph_bundle].n;
    
    initial_x_val = std::vector<values_matrix>(initial_solutions.size());
    initial_y_val = std::vector<values_matrix>(initial_solutions.size());
    
    for(auto sol_n = 0u; sol_n < initial_solutions.size(); sol_n++) {
        initial_solutions.at(sol_n).verify_feasible(g);
        
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
    
    if(DEBUG) {
        auto unfeasible_paths_n = std::count_if(g.infeas_list.begin(), g.infeas_list.end(),
            [] (const auto& kv) -> bool {
                return kv.second;
            }
        );
        
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Invoked with k_opt = " << std::boolalpha << k_opt << std::endl;
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Currently have " << unfeasible_paths_n << " precomputed unfeasible sub-paths" << std::endl;
    }

    auto total_bb_nodes_explored = (long)0;
    auto number_of_cuts_added_at_root = (long)0;
    auto time_spent_at_root = 0.0;
    auto ub_at_root = 0.0;
    auto lb_at_root = 0.0;
    auto ub = 0.0;
    auto lb = 0.0;
    auto total_cplex_time = 0.0;
    
    global::g_total_number_of_feasibility_cuts_added = 0;
    global::g_total_number_of_subtour_cuts_added = 0;
    global::g_total_number_of_generalized_order_cuts_added = 0;
    global::g_total_time_spent_separating_cuts = 0;

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
    IloRange      k_opt_constraint(env, k_opt_rhs, IloInfinity, "k_opt_constraint");

    IloObjective obj = IloMinimize(env);

    if(DEBUG) {
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Creating model (bc_setup_model)" << std::endl;
    }

    #include <solver/bc/bc_setup_model.raw.cpp>
    
    if(DEBUG) {
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Adding constraints" << std::endl;
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
    if(params.bc.two_cycles_elim) {
        model.add(two_cycles_elimination);
    }
    if(params.bc.subpath_elim) {
        model.add(subpath_elimination);
    }
    if(k_opt) {
        model.add(k_opt_constraint);
    }
    
    IloCplex cplex(model);
    
    if(DEBUG) {
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Adding initial solutions (there are " << initial_solutions.size() << " of them)" << std::endl;
    }
     
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

        // IloConstraintArray constraints(env);
        // constraints.add(outdegree); constraints.add(indegree);
        // constraints.add(y_upper); constraints.add(y_lower);
        // constraints.add(load); constraints.add(initial_load);
        // if(params.bc.two_cycles_elim) { constraints.add(two_cycles_elimination); }
        // if(k_opt) { constraints.add(k_opt_constraint); }
        //
        // IloNumArray preferences(env);
        // for(auto i = 0; i < constraints.getSize(); i++) { preferences.add(1.0); }
        //
        // if(cplex.refineMIPStartConflict(0, constraints, preferences)) {
        //     IloCplex::ConflictStatusArray conflict = cplex.getConflict(constraints);
        //     env.getImpl()->useDetailedDisplay(IloTrue);
        //     for(auto i = 0; i < constraints.getSize(); i++) {
        //         if(conflict[i] == IloCplex::ConflictMember) {
        //             std::cout << "Proven conflict: " << constraints[i] << std::endl;
        //         }
        //         if(conflict[i] == IloCplex::ConflictPossibleMember) {
        //             std::cout << "Possible conflict: " << constraints[i] << std::endl;
        //         }
        //     }
        // }

        initial_values.end();
        initial_vars.end();
    }

    // Add callbacks to separate cuts
    auto gr_with_reverse = g.make_reverse_tsp_graph();
    cplex.use(cuts_lazy_constraint_handle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
    cplex.use(cuts_callback_handle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS), params));
    
    // Add callback to print graphviz stuff
    if(!k_opt && params.bc.print_relaxation_graph) {
        cplex.use(print_relaxation_graph_callback_handle(env, variables_x, variables_y, instance_name, g));
    }

    // Export model to file
    if(!k_opt) {
        cplex.exportModel("model.lp");
    }
    
    // Set CPLEX parameters
    cplex.setParam(IloCplex::TiLim, 3600);
    cplex.setParam(IloCplex::Threads, 4);
    cplex.setParam(IloCplex::NodeLim, 0);
    
    if(k_opt && !DEBUG) {
        cplex.setOut(env.getNullStream());
    }
        
    auto t_start = high_resolution_clock::now();

    if(DEBUG) {
        std::cerr << "[DBG] bc_solver.cpp::solve() \t Solving" << std::endl;
    }

    // Solve root node
    if(!cplex.solve()) {
        if(k_opt) {
            std::cerr << "bc_solver.cpp::solve() \t CPLEX doing k-opt " << instance_name << std::endl;
        }
        std::cerr << "bc_solver.cpp::solve() \t CPLEX problem encountered at root node" << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t CPLEX status: " << cplex.getStatus() << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t CPLEX ext status: " << cplex.getCplexStatus() << std::endl;
        
        cplex.exportModel("model_err.lp");
        throw std::runtime_error("Some error occurred or the problem is infeasible");
    } else {
        auto t_end = high_resolution_clock::now();
        auto time_span = duration_cast<duration<double>>(t_end - t_start);
        
        time_spent_at_root = time_span.count();
        number_of_cuts_added_at_root =
            global::g_total_number_of_feasibility_cuts_added +
            global::g_total_number_of_subtour_cuts_added +
            global::g_total_number_of_generalized_order_cuts_added;
        lb_at_root = cplex.getBestObjValue();
        ub_at_root = cplex.getObjValue();

        // Solve the rest of the BB tree
        cplex.setParam(IloCplex::NodeLim, 2100000000);
        if(!cplex.solve()) {
            if(k_opt) {
                std::cerr << "bc_solver.cpp::solve() \t CPLEX doing k-opt" << instance_name << std::endl;
            }            
            std::cerr << "bc_solver.cpp::solve() \t CPLEX problem encountered after the root node" << std::endl;
            std::cerr << "bc_solver.cpp::solve() \t CPLEX status: " << cplex.getStatus() << std::endl;
            std::cerr << "bc_solver.cpp::solve() \t CPLEX ext status: " << cplex.getCplexStatus() << std::endl;
            
            cplex.exportModel("model_err.lp");
            throw std::runtime_error("Some error occurred or the problem is infeasible");
        }
    }

    auto t_end_total = high_resolution_clock::now();
    auto time_span_total = duration_cast<duration<double>>(t_end_total - t_start);

    if(!k_opt) {
        IloAlgorithm::Status status = cplex.getStatus();
        std::cerr << "bc_solver.cpp::solve() \t CPLEX status: " << status << std::endl;
        std::cerr << "bc_solver.cpp::solve() \t Objective value: " << cplex.getObjValue() << std::endl;
    }

    total_bb_nodes_explored = cplex.getNnodes();
    lb = cplex.getBestObjValue();
    ub = cplex.getObjValue();
    total_cplex_time = time_span_total.count();
    
    if(k_opt) { global::g_total_time_spent_by_heuristics += total_cplex_time; }
    
    IloNumArray x(env);
    IloNumArray y(env);
    IloNumArray t(env);

    cplex.getValues(x, variables_x);
    cplex.getValues(y, variables_y);

    if(!k_opt) {
        std::cerr << "bc_solver.cpp::solve() \t Solution:" << std::endl;
    }

    // Get solution
    auto col_index = 0;
    auto solution_x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    for(auto i = 0; i <= 2 * n + 1; i++) {
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(g.cost[i][j] >= 0) {
                if(x[col_index] > 0.0001) {
                    solution_x[i][j] = 1;
                    if(!k_opt) { std::cerr << "\tx(" << i << ", " << j << ") = " << x[col_index] << std::endl; }
                }
                if(y[col_index] > 0) {
                    if(!k_opt) { std::cerr << "\ty(" << i << ", " << j << ") = " << y[col_index] << std::endl; }
                }
                col_index++;
            }
         }
    }
    
    auto opt_solution_path = path(g, solution_x);
    opt_solution_path.verify_feasible(g);

    // Print solution
    if(!k_opt) {
        auto best_solution = *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const path& p1, const path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
        
        std::ofstream results_file;
        results_file.open(params.bc.results_dir + "results.txt", std::ios::out | std::ios::app);
        
        results_file << instance_name << "\t";
        results_file << total_cplex_time << "\t";
        results_file << global::g_total_time_spent_by_heuristics << "\t";
        results_file << global::g_total_time_spent_separating_cuts << "\t";
        results_file << time_spent_at_root << "\t";
        results_file << best_solution.total_cost << "\t";
        results_file << ub << "\t";
        results_file << lb << "\t";
        results_file << (ub - lb) / ub << "\t";
        results_file << ub_at_root << "\t";
        results_file << lb_at_root << "\t";
        results_file << (ub_at_root - lb_at_root) / ub_at_root << "\t";
        results_file << global::g_total_number_of_feasibility_cuts_added << "\t";
        results_file << global::g_total_number_of_subtour_cuts_added << "\t";
        results_file << global::g_total_number_of_generalized_order_cuts_added << "\t";
        results_file << global::g_total_number_of_capacity_cuts_added << "\t";
        results_file << number_of_cuts_added_at_root << "\t";
        results_file << total_bb_nodes_explored << std::endl;
        
        results_file.close();
    }

    env.end();
        
    return opt_solution_path;
}