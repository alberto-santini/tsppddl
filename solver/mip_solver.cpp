#include <solver/check_incumbent_callback.h>
#include <solver/flow_cut_callback.h>
#include <solver/mip_solver.h>

#include <ilcplex/ilocplex.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ratio>
#include <stdexcept>

MipSolver::MipSolver(const std::shared_ptr<const Graph> g, const std::vector<Path> initial_solutions, const std::string instance_name) : g{g}, initial_solutions{initial_solutions}, instance_name{instance_name} {
    find_best_initial_solution();

    std::vector<int> ip {initial_solution.path}, il {initial_solution.load};
    int n {g->g[graph_bundle].n};

    initial_solution.verify_feasible(g);

    if(initial_solution.total_cost > 0) {
        initial_x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
        initial_y = std::vector<int>(2 * n + 2, 0);
        initial_t = std::vector<int>(2 * n + 2, 0);

        for(int l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) { initial_x[ip[l]][ip[l+1]] = 1; }
            initial_y[ip[l]] = il[l];
            initial_t[ip[l]] = l;
        }
        initial_t[2 * n + 2] = 2 * n + 2;
    }
}

void MipSolver::find_best_initial_solution() {
    initial_solution = *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
}

void MipSolver::solve(const bool include_mtz) const {
    using namespace std::chrono;

    extern long g_total_number_of_cuts_added;
    extern double g_total_time_spent_by_heuristics;
    extern double g_total_time_spent_separating_cuts;
    extern long g_search_for_cuts_every_n_nodes;

    long total_bb_nodes_explored;
    long number_of_cuts_added_at_root;
    double time_spent_at_root;
    double ub_at_root;
    double lb_at_root;
    double ub;
    double lb;
    double total_cplex_time;

    g_total_number_of_cuts_added = 0;
    g_total_time_spent_separating_cuts = 0;

    int n {g->g[graph_bundle].n};
    int Q {g->g[graph_bundle].capacity};
    demand_t d {g->demand};
    draught_t l {g->draught};
    cost_t c {g->cost};

    IloEnv env;
    IloModel model(env);

    IloNumVarArray variables_x(env);
    IloNumVarArray variables_y(env);
    IloNumVarArray variables_t(env);
    IloRangeArray outdegree_constraints(env);
    IloRangeArray indegree_constraints(env);
    IloRangeArray capacity_constraints(env);
    IloRangeArray load_constraints(env);
    IloRangeArray initial_load_constraint(env);
    IloRangeArray mtz_constraints(env);
    IloRangeArray precedence_constraints(env);
    IloRangeArray fix_t(env); // Fixes t(0) to 0

    IloObjective obj = IloMinimize(env);

    /******************************** ROWS ********************************/

    for(int i = 0; i <= 2 * n; i++) {
        // 1.0 <= sum(j in i+, x(i,j)) <= 1.0
        outdegree_constraints.add(IloRange(env, 1.0, 1.0));
    }
    for(int i = 1; i <= 2 * n + 1; i++) {
        // 1.0 <= sum(j in i-, x(i,j)) <= 1.0
        indegree_constraints.add(IloRange(env, 1.0, 1.0));
    }
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                // y(i,j) - min(Q, l(i), l(j)) x(i,j) <=  0.0
                capacity_constraints.add(IloRange(env, -IloInfinity, 0.0));
            }
        }
    }
    for(int i = 1; i <= 2 * n; i++) {
        // d(i) <= sum(j in i+, y(i,j)) - sum(j in i-, y(j,i)) <= d(i)
        load_constraints.add(IloRange(env, d[i], d[i]));
    }
    // 0.0 <= sum(i in 0+, y(0,i)) <= 0.0
    initial_load_constraint.add(IloRange(env, 0.0, 0.0));

    if(include_mtz) {
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(c[i][j] >= 0) {
                    // t(i) - t(j) + (2n+1) x(i,j) <= 2n
                    mtz_constraints.add(IloRange(env, -IloInfinity, 2 * n));
                }
            }
        }
        for(int i = 1; i <= n; i++) {
            // 1.0 <= t(n+i) - t(i)
            precedence_constraints.add(IloRange(env, 1.0, IloInfinity));
        }
        // 0.0 <= t(0) <= 0.0
        fix_t.add(IloRange(env, 0.0, 0.0));
    }

    /******************************** COLUMNS X ********************************/

    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                // Set coefficient values for x(i, j) where (i, j) is an arc

                int arc_cost = c[i][j]; // c(i, j)
                IloNumColumn col = obj(arc_cost);

                // Out degree
                for(int ii = 0; ii < 2 * n + 1; ii++) {
                    // Set coefficient value for the constraint associated to ii

                    int od_coeff = 0;
                    if(i == ii) { od_coeff = 1; } // It's 1 for j in i+, for i in 0...2n
                    col += outdegree_constraints[ii](od_coeff);    // There are constraints 0...2n, corresponding to 0...2n
                }

                // In degree
                for(int jj = 1; jj <= 2 * n + 1; jj++) {
                    // Set the coefficient value for the constraint associated to jj

                    int id_coeff = 0;
                    if(j == jj) { id_coeff = 1; } // It's 1 for j in i-, for i in 1...2n+1
                    col += indegree_constraints[jj - 1](id_coeff); // There are constraints 0...2n, corresponding to 1...2n+1
                }

                // Capacity
                int c_number = 0;
                for(int ii = 0; ii <= 2 * n + 1; ii++) {
                    for(int jj = 0; jj <= 2 * n + 1; jj++) {
                        if(c[ii][jj] >= 0) {
                            // Set the coefficient value for the constraint associated with arc (ii, jj)

                            int c_coeff = 0;
                            if(i == ii && j == jj) { c_coeff = -std::min({Q, l[i], l[j]}); }
                            col += capacity_constraints[c_number++](c_coeff); // There are constraints 0...number_of_arcs
                        }
                    }
                }

                // Load
                for(int ii = 1; ii < 2 * n + 1; ii++) {
                    // Set the coefficient value for the constraint associated to ii

                    int l_coeff = 0; // x(i,j) is not involved in load constraints
                    col += load_constraints[ii - 1](l_coeff); // There are constraints 0...2n-1, corresponding to 1...2n
                }

                // Initial load
                int il_coeff = 0; // x(i,j) is not involved in initial load constraints
                col += initial_load_constraint[0](il_coeff); // There is just 1 constraint

                if(include_mtz) {
                    // MTZ
                    c_number = 0;
                    for(int ii = 0; ii <= 2 * n + 1; ii++) {
                        for(int jj = 0; jj <= 2 * n + 1; jj++) {
                            if(c[ii][jj] >= 0) {
                                // Set the coefficient value for the constraint associated to arc (ii, jj)

                                int mtz_coeff = 0;
                                if(i == ii && j == jj) { mtz_coeff = 2 * n + 1; }
                                col += mtz_constraints[c_number++](mtz_coeff); // There are constraints 0...2n+1,2n+2...4n+3,...
                            }
                        }
                    }

                    // Precedence
                    for(int ii = 1; ii <= n; ii++) {
                        // Set the coefficient value for the constraint associated with ii

                        int p_coeff = 0; // x(i,j) is not involved in precedence constraints
                        col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
                    }

                    // Fix t
                    int ft_coeff = 0; // x(i,j) is not involved in fixing t
                    col += fix_t[0](ft_coeff);
                }

                // Create the column
                IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool, ("x_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                variables_x.add(v);
            }
        }
    }

    /******************************** COLUMNS Y ********************************/

    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                // Set coefficient values for y(i, j) where (i, j) is an arc

                double arc_cost = 0; // y columns don't contribute to the obj cost
                IloNumColumn col = obj(arc_cost);

                // Out degree
                for(int ii = 0; ii < 2 * n + 1; ii++) {
                    // Set coefficient value for the constraint associated to ii

                    int od_coeff = 0; // y(i, j) is not involved in out degree constraints
                    col += outdegree_constraints[ii](od_coeff); // There are constraints 0...2n, corresponding to 0...2n
                }

                // In degree
                for(int jj = 1; jj <= 2 * n + 1; jj++) {
                    // Set the coefficient value for the constraint associated to jj

                    int id_coeff = 0; // y(i, j) is not involved in out degree constraints
                    col += indegree_constraints[jj - 1](id_coeff); // There are constraints 0...2n, corresponding to 1...2n+1
                }

                // Capacity
                int c_number = 0;
                for(int ii = 0; ii <= 2 * n + 1; ii++) {
                    for(int jj = 0; jj <= 2 * n + 1; jj++) {
                        if(c[ii][jj] >= 0) {
                            // Set the coefficient value for the constraint associated with arc (ii, jj)

                            int c_coeff = 0;
                            if(i == ii && j == jj) { c_coeff = 1; }
                            col += capacity_constraints[c_number++](c_coeff); // There are constraints 0...number_of_arcs
                        }
                    }
                }

                // Load
                for(int ii = 1; ii < 2 * n + 1; ii++) {
                    // Set the coefficient value for the constraint associated to ii

                    int l_coeff = 0;
                    if(i == ii) { l_coeff = 1; }
                    if(j == ii) { l_coeff = -1; }
                    col += load_constraints[ii - 1](l_coeff); // There are constraints 0...2n-1, corresponding to 1...2n
                }

                // Initial load
                int il_coeff = 0;
                if(i == 0) { il_coeff = 1; }
                col += initial_load_constraint[0](il_coeff); // There is just 1 constraint

                if(include_mtz) {
                    // MTZ
                    c_number = 0;
                    for(int ii = 0; ii <= 2 * n + 1; ii++) {
                        for(int jj = 0; jj <= 2 * n + 1; jj++) {
                            if(c[ii][jj] >= 0) {
                                // Set the coefficient value for the constraint associated to arc (ii, jj)

                                int mtz_coeff = 0; // y(i, j) is not involved in mtz constraints
                                col += mtz_constraints[c_number++](mtz_coeff); // There are constraints 0...2n+1,2n+2...4n+3,...
                            }
                        }
                    }

                    // Precedence
                    for(int ii = 1; ii <= n; ii++) {
                        // Set the coefficient value for the constraint associated with ii

                        int p_coeff = 0; // y(i,j) is not involved in precedence constraints
                        col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
                    }

                    // Fix t
                    int ft_coeff = 0; // y(i,j) is not involved in fixing t
                    col += fix_t[0](ft_coeff);
                }

                // Create the column
                int min_y_value {0};
                if(i >= 1 && i <= n) {
                    min_y_value = d[i];
                }
                if(j >= n+1 && j <= 2*n && -d[j] > min_y_value) {
                    min_i_value = -d[j];
                }
                
                IloNumVar v(col, min_y_value, Q, IloNumVar::Int, ("y_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                variables_y.add(v);
            }
        }
    }

    /******************************** COLUMNS T ********************************/

    if(include_mtz) {
        for(int i = 0; i <= 2 * n + 1; i++) {
            // Set coefficient values for t(i)

            double arc_cost = 0; // t columns don't contribute to the obj cost
            IloNumColumn col = obj(arc_cost);

            // Out degree
            for(int ii = 0; ii < 2 * n + 1; ii++) {
                // Set coefficient value for the constraint associated to ii

                int od_coeff = 0; // t(i) is not involved in out degree constraints
                col += outdegree_constraints[ii](od_coeff); // There are constraints 0...2n, corresponding to 0...2n
            }

            // In degree
            for(int jj = 1; jj <= 2 * n + 1; jj++) {
                // Set the coefficient value for the constraint associated to jj

                int id_coeff = 0; // t(i) is not involved in out degree constraints
                col += indegree_constraints[jj - 1](id_coeff); // There are constraints 0...2n, corresponding to 1...2n+1
            }

            // Capacity
            int c_number = 0;
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(c[ii][jj] >= 0) {
                        // Set the coefficient value for the constraint associated with arc (ii, jj)

                        int c_coeff = 0; // t(i) is not involved in capacity constraints
                        col += capacity_constraints[c_number++](c_coeff); // There are constraints 0...number_of_arcs
                    }
                }
            }

            // Load
            for(int ii = 1; ii < 2 * n + 1; ii++) {
                // Set the coefficient value for the constraint associated to ii

                int l_coeff = 0; // t(i) is not involved in load constraints
                col += load_constraints[ii - 1](l_coeff); // There are constraints 0...2n-1, corresponding to 1...2n
            }

            // Initial load
            int il_coeff = 0; // t(i) is not involved in initial load constraints
            col += initial_load_constraint[0](il_coeff); // There is just 1 constraint

            // MTZ
            c_number = 0;
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(c[ii][jj] >= 0) {
                        // Set the coefficient value for the constraint associated to arc (ii, jj)

                        int mtz_coeff = 0;
                        if(i == ii) { mtz_coeff = 1; }
                        if(i == jj) { mtz_coeff = -1; }
                        col += mtz_constraints[c_number++](mtz_coeff); // There are constraints 0...2n+1,2n+2...4n+3,...
                    }
                }
            }

            // Precedence
            for(int ii = 1; ii <= n; ii++) {
                // Set the coefficient value for the constraint associated with ii

                int p_coeff = 0;
                if(i == ii) { p_coeff = -1; }
                if(i == ii + n) { p_coeff = 1; }
                col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
            }

            // Fix t
            int ft_coeff = 0;
            if(i == 0) { ft_coeff = 1; }
            col += fix_t[0](ft_coeff);

            // Create the column
            IloNumVar v(col, 0.0, 2 * n + 1, IloNumVar::Int, ("t_{" + std::to_string(i) + "}").c_str());
            variables_t.add(v);
        }
    }

    /****************************************************************/

    model.add(obj);
    model.add(outdegree_constraints);
    model.add(indegree_constraints);
    model.add(capacity_constraints);
    model.add(load_constraints);
    model.add(initial_load_constraint);
    if(include_mtz) {
        model.add(mtz_constraints);
        model.add(precedence_constraints);
        model.add(fix_t); // Fixes t(0) to 0
    }

    IloCplex cplex(model);

    std::cout << "***** CPLEX model created" << std::endl;

    // Add initial integer solution, if present
    if(initial_solution.total_cost > 0) {
        IloNumVarArray initial_vars_x(env);
        IloNumArray initial_values_x(env);
        IloNumVarArray initial_vars_y(env);
        IloNumArray initial_values_y(env);
        IloNumVarArray initial_vars_t(env);
        IloNumArray initial_values_t(env);

        int x_idx = 0;
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(c[i][j] >= 0) {
                    initial_vars_x.add(variables_x[x_idx++]);
                    initial_values_x.add(initial_x[i][j]);
                }
            }
            initial_vars_y.add(variables_y[i]);
            initial_values_y.add(initial_y[i]);
            if(include_mtz) {
                initial_vars_t.add(variables_t[i]);
                initial_values_t.add(initial_t[i]);
            }
        }

        cplex.addMIPStart(initial_vars_x, initial_values_x);
        cplex.addMIPStart(initial_vars_y, initial_values_y);
        if(include_mtz) {
            cplex.addMIPStart(initial_vars_t, initial_values_t);
        }

        initial_values_x.end(); initial_values_y.end(); initial_values_t.end();

        std::cout << "***** Initial solution added" << std::endl;
    }

    std::shared_ptr<const Graph> gr_with_reverse {std::make_shared<const Graph>(g->make_reverse_graph())};

    if(g_search_for_cuts_every_n_nodes > 0) {
        cplex.use(FlowCutCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS), include_mtz));
        std::cout << "***** Branch and cut callback added" << std::endl;
    }

    if(!include_mtz) {
        cplex.use(CheckIncumbentCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
        std::cout << "***** Incumbent check callback added" << std::endl;
    }

    cplex.exportModel("model.lp");
    cplex.setParam(IloCplex::TiLim, 3600);
    // cplex.setParam(IloCplex::CutsFactor, 10);
    cplex.setParam(IloCplex::Threads, 4);
    cplex.setParam(IloCplex::NodeLim, 0); // Solve only the root node at first

    // DEBUG ONLY:
    // cplex.setParam(IloCplex::DataCheck, 1);

    high_resolution_clock::time_point t_start {high_resolution_clock::now()};

    if(!cplex.solve()) {
        IloAlgorithm::Status status = cplex.getStatus();
        std::cout << "CPLEX problem encountered at root node." << std::endl;
        std::cout << "CPLEX status: " << status << std::endl;
        throw std::runtime_error("Some error occurred or the problem is infeasible.");
    } else {
        high_resolution_clock::time_point t_end {high_resolution_clock::now()};
        duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};

        time_spent_at_root = time_span.count();
        number_of_cuts_added_at_root = g_total_number_of_cuts_added;
        lb_at_root = cplex.getBestObjValue();
        ub_at_root = cplex.getObjValue();

        cplex.setParam(IloCplex::NodeLim, 2100000000);
        if(!cplex.solve()) {
            IloAlgorithm::Status status = cplex.getStatus();
            std::cout << "CPLEX status: " << status << std::endl;
            throw std::runtime_error("Some error occurred or the problem is infeasible.");
        }
    }

    high_resolution_clock::time_point t_end_total {high_resolution_clock::now()};
    duration<double> time_span_total {duration_cast<duration<double>>(t_end_total - t_start)};

    IloAlgorithm::Status status = cplex.getStatus();
    std::cout << "CPLEX status: " << status << std::endl;
    std::cout << "\tObjective value: " << cplex.getObjValue() << std::endl;

    total_bb_nodes_explored = cplex.getNnodes();
    lb = cplex.getBestObjValue();
    ub = cplex.getObjValue();
    total_cplex_time = time_span_total.count();

    IloNumArray x(env);
    IloNumArray y(env);
    IloNumArray t(env);

    cplex.getValues(x, variables_x);
    cplex.getValues(y, variables_y);
    if(include_mtz) {
        cplex.getValues(t, variables_t);
    }

    std::cout << "CPLEX problem solved" << std::endl;

    // Print variables and path
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                if(x[col_index] > 0) {
                    std::cout << "\tx(" << i << ", " << j << ") = " << x[col_index] << std::endl;
                }
                if(y[col_index] > 0) {
                    std::cout << "\ty(" << i << ", " << j << ") = " << y[col_index] << std::endl;
                }
                col_index++;
            }
         }
         if(include_mtz) {
             std::cout << "\tt(" << i << ") = " << t[i] << std::endl;
         }
    }

    // *** MOVE THIS FROM HERE ONCE THE MEM PROBLEMS ARE SOLVED ***
    std::ofstream results_file;
    results_file.open("results.txt", std::ios::out | std::ios::app);
    results_file << instance_name << "\t";
    results_file << g_search_for_cuts_every_n_nodes << "\t";
    results_file << total_cplex_time << "\t";
    results_file << g_total_time_spent_by_heuristics << "\t";
    results_file << g_total_time_spent_separating_cuts << "\t";
    results_file << time_spent_at_root << "\t";
    results_file << ub << "\t";
    results_file << lb << "\t";
    results_file << (ub - lb) / ub << "\t";
    results_file << ub_at_root << "\t";
    results_file << lb_at_root << "\t";
    results_file << (ub_at_root - lb_at_root) / ub_at_root << "\t";
    results_file << g_total_number_of_cuts_added << "\t";
    results_file << number_of_cuts_added_at_root << "\t";
    results_file << total_bb_nodes_explored << std::endl;
    results_file.close();
    // *** END OF THE PART TO MOVE ***

    x.end(); y.end(); t.end();

    try {
        env.end();
    } catch(...) {
        std::cout << "Maybe it's my fault, but I feel like CPLEX's memory management sucks!" << std::endl;
    }
}
