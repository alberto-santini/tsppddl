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

MipSolver::MipSolver(const std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, const std::string& instance_name) : g{g}, initial_solutions{initial_solutions}, instance_name{instance_name} {
    find_best_initial_solution();
    initial_solution.verify_feasible(g);
    int n {g->g[graph_bundle].n};
    
    initial_x_val = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    initial_y_val = std::vector<int>(2 * n + 2, 0);
    initial_t_val = std::vector<int>(2 * n + 2, 0);

    if(initial_solution.total_cost > 0) {
        for(int l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) { initial_x_val[initial_solution.path[l]][initial_solution.path[l+1]] = 1; }
            initial_y_val[initial_solution.path[l]] = initial_solution.load[l];
            initial_t_val[initial_solution.path[l]] = l;
        }
        initial_t_val[2 * n + 1] = 2 * n + 1;
    }
}

void MipSolver::find_best_initial_solution() {
    initial_solution = *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
}

void MipSolver::solve_with_mtz(const bool use_valid_y_ineq) const {
    solve(true, false, false, use_valid_y_ineq, false);
}

void MipSolver::solve_with_lagrangian_relaxation_precedence(const std::vector<double>& mult_mu, const bool use_valid_y_ineq) {
    this->mult_mu = mult_mu;
    solve(true, false, true, use_valid_y_ineq, false);
}

void MipSolver::solve_with_lagrangian_relaxation_precedence_and_cycles(const std::vector<std::vector<double>>& mult_lambda, const std::vector<double>& mult_mu, const bool use_valid_y_ineq) {
    this->mult_lambda = mult_lambda;
    this->mult_mu = mult_mu;
    solve(true, true, true, use_valid_y_ineq, false);
}

void MipSolver::solve_with_branch_and_cut(const bool use_valid_y_ineq) const {
    solve(false, false, false, use_valid_y_ineq, false);
}

std::vector<std::vector<int>> MipSolver::solve_for_k_opt(const std::vector<std::vector<int>>& lhs, const int& rhs) {
    k_opt_lhs = lhs;
    k_opt_rhs = rhs;
    return solve(true, false, false, true, true);
}

std::vector<std::vector<int>> MipSolver::solve(const bool include_mtz, const bool use_lagrange_cycles, const bool use_lagrange_precedence, const bool use_valid_y_ineq, const bool k_opt) const {
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
    std::cerr << "Created env" << std::endl;
    IloModel model(env);
    std::cerr << "Created model" << std::endl;

    IloNumVarArray variables_x(env);
    IloNumVarArray variables_y(env);
    IloNumVarArray variables_tt(env);
    
    std::cerr << "Created variables" << std::endl;
    
    IloRangeArray outdegree_constraints(env);
    IloRangeArray indegree_constraints(env);
    IloRangeArray capacity_constraints(env);
    IloRangeArray valid_y_ineq(env);
    IloRangeArray load_constraints(env);
    IloRangeArray initial_load_constraint(env);
    IloRangeArray mtz_constraints(env);
    IloRangeArray precedence_constraints(env);
    IloRangeArray fix_t(env); // Fixes t(0) to 0
    IloRangeArray k_opt_constraint(env);
    
    std::cerr << "Created constraints" << std::endl;

    IloObjective obj = IloMinimize(env);
    
    std::cerr << "Created objective function" << std::endl;

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
    if(use_valid_y_ineq) {
        for(int i = 1; i <= 2 * n; i++) {
            for(int j = 1; j <= 2 * n; j++) {
                if(c[i][j] > 0 && (i <= n || j > n)) {
                    // 0 <= y(i,j) - N x(i,j)
                    valid_y_ineq.add(IloRange(env, 0.0, IloInfinity));
                }
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
        if(!use_lagrange_cycles) {
            for(int i = 0; i <= 2 * n + 1; i++) {
                for(int j = 0; j <= 2 * n + 1; j++) {
                    if(c[i][j] >= 0) {
                        // t(i) - t(j) + (2n+1) x(i,j) <= 2n
                        mtz_constraints.add(IloRange(env, -IloInfinity, 2 * n));
                    }
                }
            }
        }
        if(!use_lagrange_precedence) {
            for(int i = 1; i <= n; i++) {
                // 1.0 <= t(n+i) - t(i)
                precedence_constraints.add(IloRange(env, 1.0, IloInfinity));
            }
        }
        // 0.0 <= t(0) <= 0.0
        fix_t.add(IloRange(env, 0.0, 0.0));
        // lhs >= rhs ---> rhs <= lhs
        if(k_opt) {
            k_opt_constraint.add(IloRange(env, k_opt_rhs, IloInfinity));
        }
    }
    std::cerr << "Initialised constraints" << std::endl;

    /******************************** COLUMNS X ********************************/

    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                // Set coefficient values for x(i, j) where (i, j) is an arc

                // std::cerr << "Initialising variable x[" << i << "][" << j << "]" << std::endl;

                int arc_cost = c[i][j]; // c(i, j)
                
                if(use_lagrange_cycles) {
                    arc_cost += (2 * n + 1) * mult_lambda[i][j];
                }
                
                IloNumColumn col = obj(arc_cost);
                
                // std::cerr << "\t Added column via objective" << std::endl;

                // Out degree
                for(int ii = 0; ii < 2 * n + 1; ii++) {
                    // Set coefficient value for the constraint associated to ii

                    int od_coeff = 0;
                    if(i == ii) { od_coeff = 1; } // It's 1 for j in i+, for i in 0...2n
                    col += outdegree_constraints[ii](od_coeff);    // There are constraints 0...2n, corresponding to 0...2n
                }
                
                // std::cerr << "\t Added out degree constraints" << std::endl;

                // In degree
                for(int jj = 1; jj <= 2 * n + 1; jj++) {
                    // Set the coefficient value for the constraint associated to jj

                    int id_coeff = 0;
                    if(j == jj) { id_coeff = 1; } // It's 1 for j in i-, for i in 1...2n+1
                    col += indegree_constraints[jj - 1](id_coeff); // There are constraints 0...2n, corresponding to 1...2n+1
                }
                
                // std::cerr << "\t Added in degree constraints" << std::endl;

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
                
                // std::cerr << "\t Added capacity constraints" << std::endl;
                
                if(use_valid_y_ineq) {
                    // Valid y inequalities
                    c_number = 0;
                    for(int ii = 1; ii <= 2 * n; ii++) {
                        for(int jj = 1; jj <= 2 * n; jj++) {
                            if(c[ii][jj] > 0 && (ii <= n || jj > n)) {
                                int my_coeff {0};
                            
                                if(i == ii && j == jj) {
                                    if(ii <= n) { my_coeff -= d[ii]; }
                                    if(jj > n && jj != ii + n) { my_coeff += d[jj]; }
                                }
                            
                                col += valid_y_ineq[c_number++](my_coeff);
                            }
                        }
                    }
                    // std::cerr << "\t Added valid y ineq constraints" << std::endl;
                }
                            

                // Load
                for(int ii = 1; ii < 2 * n + 1; ii++) {
                    // Set the coefficient value for the constraint associated to ii

                    int l_coeff = 0; // x(i,j) is not involved in load constraints
                    col += load_constraints[ii - 1](l_coeff); // There are constraints 0...2n-1, corresponding to 1...2n
                }
                
                // std::cerr << "\t Added load constraints" << std::endl;

                // Initial load
                int il_coeff = 0; // x(i,j) is not involved in initial load constraints
                col += initial_load_constraint[0](il_coeff); // There is just 1 constraint
                
                // std::cerr << "\t Added the initial load constraint" << std::endl;

                if(include_mtz) {
                    // MTZ
                    c_number = 0;
                    if(!use_lagrange_cycles) {
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
                        
                        // std::cerr << "\t Added mtz constraints" << std::endl;
                    }
                    

                    // Precedence
                    if(!use_lagrange_precedence) {
                        for(int ii = 1; ii <= n; ii++) {
                            // Set the coefficient value for the constraint associated with ii

                            int p_coeff = 0; // x(i,j) is not involved in precedence constraints
                            col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
                        }
                        
                        // std::cerr << "\t Added precedence constraints" << std::endl;
                    }

                    // Fix t
                    int ft_coeff = 0; // x(i,j) is not involved in fixing t
                    col += fix_t[0](ft_coeff);
                    // std::cerr << "\t Added the fix_t constraint" << std::endl;
                }
                
                if(k_opt) {
                    int ko_coeff {k_opt_lhs[i][j]};
                    col += k_opt_constraint[0](ko_coeff);
                    // std::cerr << "\t Added the k_opt constraint" << std::endl;
                }

                // Create the column
                IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool);//, ("x_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                
                // std::cerr << "IloNumVar created" << std::endl;
                
                variables_x.add(v);
                
                // std::cerr << "IloNumVar added to the variables" << std::endl;
            }
        }
    }
    std::cerr << "Initialised variables x" << std::endl;

    /******************************** COLUMNS Y ********************************/

    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                // Set coefficient values for y(i, j) where (i, j) is an arc

                double var_cost = 0; // y columns don't contribute to the obj cost
                IloNumColumn col = obj(var_cost);

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
                
                if(use_valid_y_ineq) {
                    // Valid y inequalities
                    c_number = 0;
                    for(int ii = 1; ii <= 2 * n; ii++) {
                        for(int jj = 1; jj <= 2 * n; jj++) {
                            if(c[ii][jj] > 0 && (ii <= n || jj > n)) {
                                int my_coeff {0};
                            
                                if(i == ii && j == jj) {
                                    my_coeff = 1;
                                }
                            
                                col += valid_y_ineq[c_number++](my_coeff);
                            }
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
                    if(!use_lagrange_cycles) {
                        for(int ii = 0; ii <= 2 * n + 1; ii++) {
                            for(int jj = 0; jj <= 2 * n + 1; jj++) {
                                if(c[ii][jj] >= 0) {
                                    // Set the coefficient value for the constraint associated to arc (ii, jj)

                                    int mtz_coeff = 0; // y(i, j) is not involved in mtz constraints
                                    col += mtz_constraints[c_number++](mtz_coeff); // There are constraints 0...2n+1,2n+2...4n+3,...
                                }
                            }
                        }
                    }

                    // Precedence
                    if(!use_lagrange_precedence) {
                        for(int ii = 1; ii <= n; ii++) {
                            // Set the coefficient value for the constraint associated with ii

                            int p_coeff = 0; // y(i,j) is not involved in precedence constraints
                            col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
                        }
                    }

                    // Fix t
                    int ft_coeff = 0; // y(i,j) is not involved in fixing t
                    col += fix_t[0](ft_coeff);
                }
                
                if(k_opt) {
                    int ko_coeff {0};
                    col += k_opt_constraint[0](ko_coeff);
                }

                // Create the column
                IloNumVar v(col, 0.0, Q, IloNumVar::Int);//, ("y_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                variables_y.add(v);
            }
        }
    }
    std::cerr << "Initialised variables y" << std::endl;

    /******************************** COLUMNS T ********************************/

    if(include_mtz) {
        for(int i = 0; i <= 2 * n + 1; i++) {
            // Set coefficient values for t(i)

            double var_cost = 0; // t columns don't contribute to the obj cost
            
            if(use_lagrange_cycles) {
                for(int j = 0; j <= 2 * n + 1; j++) {
                    if(c[i][j] >= 0) {
                        var_cost -= mult_lambda[i][j];
                    }
                    if(c[j][i] >= 0) {
                        var_cost += mult_lambda[j][i];
                    }
                }
            }
            
            if(use_lagrange_precedence) {
                if(i >= 1 && i <= n) {
                    var_cost -= mult_mu[i];
                    var_cost += mult_mu[n+i];
                }
            }
            
            IloNumColumn col = obj(var_cost);

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
            
            if(use_valid_y_ineq) {
                // Valid y inequalities
                c_number = 0;
                for(int ii = 1; ii <= 2 * n; ii++) {
                    for(int jj = 1; jj <= 2 * n; jj++) {
                        if(c[ii][jj] > 0 && (ii <= n || jj > n)) {
                            int my_coeff {0};
                            col += valid_y_ineq[c_number++](my_coeff);
                        }
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
            if(!use_lagrange_cycles) {
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
            }

            // Precedence
            if(!use_lagrange_precedence) {
                for(int ii = 1; ii <= n; ii++) {
                    // Set the coefficient value for the constraint associated with ii

                    int p_coeff = 0;
                    if(i == ii) { p_coeff = -1; }
                    if(i == ii + n) { p_coeff = 1; }
                    col += precedence_constraints[ii - 1](p_coeff); // There are constraints 0...n-1, corresponding to 1...n
                }
            }

            // Fix t
            int ft_coeff = 0;
            if(i == 0) { ft_coeff = 1; }
            col += fix_t[0](ft_coeff);
            
            if(k_opt) {
                int ko_coeff {0};
                col += k_opt_constraint[0](ko_coeff);
            }

            // Create the column
            IloNumVar v(col, 0.0, 2 * n + 1, IloNumVar::Int);//, ("t_{" + std::to_string(i) + "}").c_str());
            variables_tt.add(v);
        }
        std::cerr << "Initialised variables t" << std::endl;
    }

    /****************************************************************/

    model.add(obj);
    
    std::cerr << "Added objective function to model" << std::endl;
    
    model.add(outdegree_constraints);
    model.add(indegree_constraints);
    model.add(capacity_constraints);
    if(use_valid_y_ineq) {
        model.add(valid_y_ineq);
    }
    model.add(load_constraints);
    model.add(initial_load_constraint);
    if(include_mtz) {
        if(!use_lagrange_cycles) { model.add(mtz_constraints); }
        if(!use_lagrange_precedence) { model.add(precedence_constraints); }
        model.add(fix_t); // Fixes t(0) to 0
    }
    if(k_opt) {
        model.add(k_opt_constraint);
    }
    
    std::cerr << "Added constraints to model" << std::endl;

    IloCplex cplex(model);
    
    std::cerr << "Created cplex" << std::endl;

    std::cout << "***** CPLEX model created" << std::endl;

    // Add initial integer solution, if present
    if(initial_solution.total_cost > 0) {
        IloNumVarArray initial_x_vars(env);
        IloNumArray initial_x_values(env);
        IloNumVarArray initial_y_vars(env);
        IloNumArray initial_y_values(env);
        IloNumVarArray initial_t_vars(env);
        IloNumArray initial_t_values(env);
        
        std::cerr << "Created IloNumVarArray and IloNumArray for the initial solution" << std::endl;

        int x_idx = 0;
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(c[i][j] >= 0) {
                    initial_x_vars.add(variables_x[x_idx++]);
                    initial_x_values.add(initial_x_val[i][j]);
                }
            }
            initial_y_vars.add(variables_y[i]);
            initial_y_values.add(initial_y_val[i]);
            if(include_mtz) {
                initial_t_vars.add(variables_tt[i]);
                initial_t_values.add(initial_t_val[i]);
            }
        }
        
        std::cerr << "Initialised initial variables" << std::endl;

        cplex.addMIPStart(initial_x_vars, initial_x_values);
        cplex.addMIPStart(initial_y_vars, initial_y_values);
        if(include_mtz) {
            cplex.addMIPStart(initial_t_vars, initial_t_values);
        }
        
        std::cerr << "Added mip start" << std::endl;

        initial_x_values.end(); initial_y_values.end(); initial_t_values.end();
        
        std::cerr << ".end()-ed the initial values IloNumArray" << std::endl;

        std::cout << "***** Initial solution added" << std::endl;
    }

    if(!include_mtz) {
        // Then it means we are doing branch_and_cut!
        std::shared_ptr<const Graph> gr_with_reverse {std::make_shared<const Graph>(g->make_reverse_graph())};

        if(g_search_for_cuts_every_n_nodes > 0) {
            cplex.use(FlowCutCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS), include_mtz));
            std::cout << "***** Branch and cut callback added" << std::endl;
            std::cerr << "Added FlowCutCallbackHandle" << std::endl;
        }

        if(!include_mtz) {
            cplex.use(CheckIncumbentCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
            std::cout << "***** Incumbent check callback added" << std::endl;
            std::cerr << "Added CheckIncumbentCallbackHandle" << std::endl;
        }
    }
    
    double missing_from_obj_value {0};
    if(use_lagrange_cycles) {
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(c[i][j] >= 0) {
                    missing_from_obj_value -= mult_lambda[i][j];
                }
            }
        }
    }
    if(use_lagrange_precedence) {
        for(int i = 1; i <= n; i++) {
            missing_from_obj_value -= mult_mu[i];
        }
    }

    std::string model_file_name;
    if(k_opt) {
        model_file_name = "kopt_model.lp";
    } else {
        model_file_name = "model.lp";
    }
    cplex.exportModel(model_file_name.c_str());
    
    std::cerr << "Model wrote to file" << std::endl;
    
    cplex.setParam(IloCplex::TiLim, 3600);
    // cplex.setParam(IloCplex::CutsFactor, 10);
    cplex.setParam(IloCplex::Threads, 4);
    cplex.setParam(IloCplex::NodeLim, 0); // Solve only the root node at first

    // DEBUG ONLY:
    // cplex.setParam(IloCplex::DataCheck, 1);
    
    std::cerr << "Set cplex parameters" << std::endl;

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
        lb_at_root = cplex.getBestObjValue() + missing_from_obj_value;
        ub_at_root = cplex.getObjValue() + missing_from_obj_value;

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
    std::cout << "\tObjective value: " << cplex.getObjValue() + missing_from_obj_value << std::endl;

    total_bb_nodes_explored = cplex.getNnodes();
    lb = cplex.getBestObjValue() + missing_from_obj_value;
    ub = cplex.getObjValue() + missing_from_obj_value;
    total_cplex_time = time_span_total.count();

    IloNumArray x(env);
    IloNumArray y(env);
    IloNumArray t(env);

    cplex.getValues(x, variables_x);
    cplex.getValues(y, variables_y);
    if(include_mtz) {
        cplex.getValues(t, variables_tt);
    }

    std::cout << "CPLEX problem solved" << std::endl;

    // Print variables and path
    int col_index {0};
    std::vector<std::vector<int>> solution_x(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                if(x[col_index] > 0) {
                    std::cerr << "Setting solution_x[" << i << "][" << j << "]" << std::endl;
                    solution_x[i][j] = 1;
                    if(!k_opt) { std::cout << "\tx(" << i << ", " << j << ") = " << x[col_index] << std::endl; }
                }
                if(y[col_index] > 0) {
                    if(!k_opt) { std::cout << "\ty(" << i << ", " << j << ") = " << y[col_index] << std::endl; }
                }
                col_index++;
            }
         }
         if(include_mtz) {
             if(!k_opt) { std::cout << "\tt(" << i << ") = " << t[i] << std::endl; }
         }
    }

    std::ofstream results_file;
    std::string results_file_name;
    if(k_opt) {
        results_file_name = "kopt.txt";
    } else {
        results_file_name = "results.txt";
    }
    results_file.open(results_file_name, std::ios::out | std::ios::app);
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

    try {
        x.end(); y.end(); t.end(); env.end();
    } catch(...) {
        std::cout << "Error while .end()-ing!" << std::endl;
    }
    
    return solution_x;
}
