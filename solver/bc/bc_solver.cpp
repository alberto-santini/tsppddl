#include <global.h>
#include <solver/bc/callbacks/cuts_callback.h>
#include <solver/bc/callbacks/cuts_lazy_constraint.h>
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

BcSolver::BcSolver(const Graph& g, const ProgramParams& params, const std::vector<Path>& initial_solutions, const std::string& instance_path) : g{g}, params{params}, initial_solutions{initial_solutions} {
    initial_solution = find_best_initial_solution();
    initial_solution.verify_feasible(g);
    auto n = g.g[graph_bundle].n;
    
    initial_x_val = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    initial_y_val = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));

    if(initial_solution.total_cost > 0) {
        for(auto l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) {
                initial_x_val[initial_solution.path[l]][initial_solution.path[l+1]] = 1;
                initial_y_val[initial_solution.path[l]][initial_solution.path[l+1]] = initial_solution.load[l];
            }
        }
    }
    
    // PORTABLE WAY:
    // boost::filesystem::path i_path(instance_path);
    // std::stringstream ss; ss << i_path.stem();
    // instance_name = ss.str();
    
    // NOT-PORTABLE WAY:
    auto path_parts = std::vector<std::string>();
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    auto file_parts = std::vector<std::string>();
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));
    file_parts.pop_back();
    
    instance_name = boost::algorithm::join(file_parts, ".");
}

Path BcSolver::find_best_initial_solution() {
    return *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
}

std::vector<std::vector<int>> BcSolver::solve_for_k_opt(const std::vector<std::vector<int>>& lhs, int rhs) {
    k_opt_lhs = lhs;
    k_opt_rhs = rhs;
    return solve(true);
}

void BcSolver::solve_with_branch_and_cut() const {
    solve(false);
}

// Check if (i,j) -> (j,k) can be a subpath of a feasible path
// It assumes that (i,j) and (j,k) both exist
// It assumes that 1 <= i,j,k <= 2n
// It returns true if any path containing that subpath should be eliminated
bool BcSolver::is_path_eliminable(int i, int j, int k) const {
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
        
    if(i == n + k) {
        return true;
    }
    
    if(i <= n) {
        if(j <= n) {
            if(k <= n) {
                return (g.demand.at(i) + g.demand.at(j) + g.demand.at(k) > std::min(Q, g.draught.at(k)));
            } else {
                return (    k != n+i && k != n+j && (
                                (g.demand.at(i) + g.demand.at(j) + g.demand.at(k-n) >
                                    std::min(Q, std::min(g.draught.at(j), g.draught.at(k)))) ||
                                (g.demand.at(i) + g.demand.at(k-n) >
                                    std::min(Q, g.draught.at(k)))
                            )
                       );
            }
        } else {
            if(k <= n) {
                return (j != n+i && (g.demand.at(i) + g.demand.at(k) > std::min(Q, g.draught.at(k))));
            } else {
                return (    j != n+i && k != n+i && (
                                (g.demand.at(i) + g.demand.at(j-n) + g.demand.at(k-n) >
                                    std::min(Q, std::min(g.draught.at(i), g.draught.at(j)))) ||
                                (g.demand.at(i) + g.demand.at(k-n) >
                                    std::min(Q, g.draught.at(k)))
                            )
                       );
            }
        }
    } else {
        if(j <= n) {
            if(k <= n) {
                return false;
            } else {
                return (k != n+j && g.demand.at(i-n) + g.demand.at(k-n) > std::min(Q, g.draught.at(i)));
            }
        } else {
            if(k <= n) {
                return false;
            } else {
                return (g.demand.at(i-n) + g.demand.at(j-n) + g.demand.at(k-n) > std::min(Q, g.draught.at(i)));
            }
        }
    }
}

std::vector<std::vector<int>> BcSolver::solve(bool k_opt) const {
    using namespace std::chrono;

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
    IloNumVarArray variables_tt(env);
    
    IloRangeArray outdegree(env);
    IloRangeArray indegree(env);
    IloRangeArray y_upper(env);
    IloRangeArray y_lower(env);
    IloRangeArray load(env);
    IloRangeArray initial_load(env);
    IloRangeArray two_cycles_elimination(env);
    IloRangeArray subpath_elimination(env);
    IloRangeArray k_opt_constraint(env);

    IloObjective obj = IloMinimize(env);

    #include <solver/bc/bc_setup_model.raw.cpp>
    
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
    
    // Add initial solution
    if(initial_solution.total_cost > 0) {
        IloNumVarArray initial_vars(env);
        IloNumArray initial_values(env);
        
        auto col_idx = 0;
        for(auto i = 0; i <= 2 * n + 1; i++) {
            for(auto j = 0; j <= 2 * n + 1; j++) {
                if(g.cost[i][j] >= 0) {
                    initial_vars.add(variables_x[col_idx]);
                    initial_values.add(initial_x_val[i][j]);
                    initial_vars.add(variables_y[col_idx]);
                    initial_values.add(initial_y_val[i][j]);
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
    auto gr_with_reverse = g.make_reverse_graph();
    if(params.bc.cut_every_n_nodes > 0) {
        cplex.use(CutsLazyConstraintHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
        cplex.use(CutsCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS), params));
    }

    // Export model to file
    if(!k_opt) { cplex.exportModel("model.lp"); }
    
    // Set CPLEX parameters
    cplex.setParam(IloCplex::TiLim, 3600);
    cplex.setParam(IloCplex::Threads, 1);
    cplex.setParam(IloCplex::NodeLim, 0);
        
    auto t_start = high_resolution_clock::now();

    // Solve root node
    if(!cplex.solve()) {
        std::cerr << "BcSolver::solve()\tCPLEX problem encountered at root node." << std::endl;
        std::cerr << "BcSolver::solve()\tCPLEX status: " << cplex.getStatus() << std::endl;
        std::cerr << "BcSolver::solve()\tCPLEX ext status: " << cplex.getCplexStatus() << std::endl;
        cplex.exportModel("model_err.lp");
        throw std::runtime_error("Some error occurred or the problem is infeasible.");
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
            std::cerr << "BcSolver::solve()\tCPLEX problem encountered after the root node." << std::endl;
            std::cerr << "BcSolver::solve()\tCPLEX status: " << cplex.getStatus() << std::endl;
            std::cerr << "BcSolver::solve()\tCPLEX ext status: " << cplex.getCplexStatus() << std::endl;
            cplex.exportModel("model_err.lp");
            throw std::runtime_error("Some error occurred or the problem is infeasible.");
        }
    }

    auto t_end_total = high_resolution_clock::now();
    auto time_span_total = duration_cast<duration<double>>(t_end_total - t_start);

    IloAlgorithm::Status status = cplex.getStatus();
    std::cerr << "BcSolver::solve()\tCPLEX status: " << status << std::endl;
    std::cerr << "BcSolver::solve()\tObjective value: " << cplex.getObjValue() << std::endl;

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

    if(!k_opt) { std::cerr << "BcSolver::solve()\tSolution:" << std::endl; }

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

    // Print solution
    if(!k_opt) {
        std::ofstream results_file;
        results_file.open(params.bc.results_dir + "results.txt", std::ios::out | std::ios::app);
        
        results_file << instance_name << "\t";
        results_file << params.bc.cut_every_n_nodes << "\t";
        results_file << std::boolalpha << params.bc.two_cycles_elim << "\t";
        results_file << std::boolalpha << params.bc.subpath_elim << "\t";
        results_file << total_cplex_time << "\t";
        results_file << global::g_total_time_spent_by_heuristics << "\t";
        results_file << global::g_total_time_spent_separating_cuts << "\t";
        results_file << time_spent_at_root << "\t";
        results_file << initial_solution.total_cost << "\t";
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
    
    return solution_x;
}