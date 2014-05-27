#ifndef MIP_SOLVER_CPP
#define MIP_SOLVER_CPP

#include <algorithm>
#include <iostream>
#include <cstring>
#include <utility>

#include <ilcplex/ilocplex.h>

#include <solver/mip_solver.h>

MipSolver::MipSolver(std::shared_ptr<Graph> graph, std::tuple<std::vector<int>, std::vector<int>, int> initial_solution) : graph(graph), initial_solution(initial_solution) {
    std::vector<int> initial_path;
    std::vector<int> partial_load;
    int cost;
    
    std::tie(initial_path, partial_load, cost) = initial_solution;
    
    n = graph->g[graph_bundle].num_requests;
    
    if(cost >= 0) {
        std::cout << "CPLEX Initial solution present. Loading it." << std::endl;
        initial_x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
        initial_y = std::vector<int>(2 * n + 2, 0);
        initial_t = std::vector<int>(2 * n + 2, 0);
    
        for(int l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) { initial_x[initial_path[l]][initial_path[l+1]] = 1; }
            initial_y[initial_path[l]] = partial_load[l];
            initial_t[initial_path[l]] = l;
        }
        initial_t[2 * n + 2] = 2 * n + 2;
    }
}

void MipSolver::solve() const {
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
    
    std::cout << "CPLEX Creating auxiliary data" << std::endl;
    
    int Q = graph->g[graph_bundle].ship_capacity;
    std::vector<int> l = std::vector<int>(2 * n + 2, 1000); // Default draught if not specified: 1000
    std::vector<int> d = std::vector<int>(2 * n + 2, 0); // Default demand if not specified: 0
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(graph->g); vi != vi_end; ++vi) {
        std::shared_ptr<Node> node = graph->g[*vi];
        l[node->id] = node->port->draught;
        d[node->id] = node->demand;
    }
    
    std::cout << "CPLEX Constructing the model" << std::endl;
    
    // ROWS
    
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
            if(graph->handy_dt[i][j] >= 0) {
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
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(graph->handy_dt[i][j] >= 0) {
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
    
    std::cout << "CPLEX rows added" << std::endl;
    
    // COLUMNS x
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(graph->handy_dt[i][j] >= 0) {
                // Set coefficient values for x(i, j) where (i, j) is an arc
            
                double arc_cost = graph->handy_dt[i][j]; // c(i, j)
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
                        if(graph->handy_dt[ii][jj] >= 0) {
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
            
                // MTZ
                c_number = 0;
                for(int ii = 0; ii <= 2 * n + 1; ii++) {
                    for(int jj = 0; jj <= 2 * n + 1; jj++) {
                        if(graph->handy_dt[ii][jj] >= 0) {
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
                        
                // Create the column
                IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool, ("x_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                variables_x.add(v);
            }
        }
    }
    
    std::cout << "CPLEX x columns added" << std::endl;
    
    // COLUMNS y
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(graph->handy_dt[i][j] >= 0) {
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
                        if(graph->handy_dt[ii][jj] >= 0) {
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
            
                // MTZ
                c_number = 0;
                for(int ii = 0; ii <= 2 * n + 1; ii++) {
                    for(int jj = 0; jj <= 2 * n + 1; jj++) {
                        if(graph->handy_dt[ii][jj] >= 0) {
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
                        
                // Create the column
                IloNumVar v(col, 0.0, Q, IloNumVar::Int, ("y_{" + std::to_string(i) + "," + std::to_string(j) + "}").c_str());
                variables_y.add(v);
            }
        }
    }
    
    std::cout << "CPLEX y columns added" << std::endl;
    
    // COLUMNS t
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
                if(graph->handy_dt[ii][jj] >= 0) {
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
                if(graph->handy_dt[ii][jj] >= 0) {
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
    
    std::cout << "CPLEX t columns added" << std::endl;
    
    model.add(obj);
    model.add(outdegree_constraints);
    model.add(indegree_constraints);
    model.add(capacity_constraints);
    model.add(load_constraints);
    model.add(initial_load_constraint);
    model.add(mtz_constraints);
    model.add(precedence_constraints);
    model.add(fix_t); // Fixes t(0) to 0
    
    std::cout << "CPLEX model ready" << std::endl;
    
    IloCplex cplex(model);
    // cplex.setOut(env.getNullStream());
    
    if(std::get<2>(initial_solution) >= 0) {
        // Add initial integer solution
        IloNumVarArray initial_vars_x(env);
        IloNumArray initial_values_x(env);
        IloNumVarArray initial_vars_y(env);
        IloNumArray initial_values_y(env);
        IloNumVarArray initial_vars_t(env);
        IloNumArray initial_values_t(env);
        
        int x_idx = 0;
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(graph->handy_dt[i][j] >= 0) {
                    initial_vars_x.add(variables_x[x_idx++]);
                    initial_values_x.add(initial_x[i][j]);
                }
            }
            initial_vars_y.add(variables_y[i]);
            initial_values_y.add(initial_y[i]);
            initial_vars_t.add(variables_t[i]);
            initial_values_t.add(initial_t[i]);
        }
        
        cplex.addMIPStart(initial_vars_x, initial_values_x);
        cplex.addMIPStart(initial_vars_y, initial_values_y);
        cplex.addMIPStart(initial_vars_t, initial_values_t);
        
        initial_vars_x.end(); initial_vars_y.end(); initial_vars_t.end();
        
        std::cout << "CPLEX added initial MIP solution" << std::endl;
    }
    
    cplex.exportModel("model.lp");
    cplex.setParam(IloCplex::TiLim, 3600);
    
    std::cout << "CPLEX solving" << std::endl;
    
    if(!cplex.solve()) {
        IloAlgorithm::Status status = cplex.getStatus();
        std::cout << "CPLEX status: " << status << std::endl;
        throw std::runtime_error("Some error occurred or the problem is infeasible.");
    }
    
    IloAlgorithm::Status status = cplex.getStatus();
    std::cout << "CPLEX status: " << status << std::endl;
    std::cout << "\tObjective value: " << cplex.getObjValue() << std::endl;
    
    IloNumArray x(env);
    IloNumArray y(env);
    IloNumArray t(env);
    
    cplex.getValues(x, variables_x);
    cplex.getValues(y, variables_y);
    cplex.getValues(t, variables_t);
    
    std::cout << "CPLEX problem solved" << std::endl;
    
    // Print variables and path
    int col_index = 0;
    Path up;
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(graph->handy_dt[i][j] >= 0) {
                if(x[col_index] > 0) {
                    std::cout << "\tx(" << i << ", " << j << ") = " << x[col_index] << std::endl;
                    up.push_back(graph->get_edge(i, j));
                }
                if(y[col_index] > 0) {
                    std::cout << "\ty(" << i << ", " << j << ") = " << y[col_index] << std::endl;
                }
                col_index++;
            }
         }
         std::cout << "\tt(" << i << ") = " << t[i] << std::endl;
    }
    
    std::cout << "The solution (port visit order) is: ";
    graph->print_unordered_path(up);
    
    x.end(); y.end(); t.end();
    env.end();
}

#endif