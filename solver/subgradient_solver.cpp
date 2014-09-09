#include <solver/subgradient_solver.h>

#include <ilcplex/ilocplex.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

void SubgradientSolver::solve() {
    using namespace std::chrono;
    
    auto floating_equal = [] (double x, double y) -> bool {
        double diff = std::abs(x - y); x = std::abs(x); y = std::abs(y); double largest = (y > x) ? y : x;
        if(diff <= largest * 1000 * std::numeric_limits<double>::epsilon()) { return true; } else { return false; }
    };
    
    int n {g->g[graph_bundle].n};
    int Q {g->g[graph_bundle].capacity};
    demand_t d {g->demand};
    draught_t l {g->draught};
    cost_t c {g->cost};
    
    best_sol = std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); })->total_cost;
        
    std::vector<std::vector<double>> lambda(2 * n + 2, std::vector<double>(2 * n + 2, 1.0));
    std::vector<double> mu(n+1, 1.0); // I include mu[0] for convenience
    
    double best_bound {0};
    std::vector<std::vector<double>> best_lambda;
    std::vector<double> best_mu;
    
    IloEnv env;
    IloModel model(env);

    IloNumVarArray variables_x(env);
    IloNumVarArray variables_y(env);
    IloNumVarArray variables_tt(env);

    IloRangeArray outdegree(env);
    IloRangeArray indegree(env);
    IloRangeArray load(env);
    IloRangeArray y_lower(env);
    IloRangeArray y_upper(env);
    IloRangeArray initial_load(env);
    IloRangeArray initial_order(env);
    
    IloObjective obj = IloMinimize(env);
    
    #include <solver/lagrange_setup_model.raw.cpp>
    
    model.add(obj);
    model.add(variables_x); model.add(variables_y); model.add(variables_tt);
    model.add(outdegree); model.add(indegree); model.add(load); model.add(y_lower); model.add(y_upper); model.add(initial_load); model.add(initial_order);
    
    IloCplex cplex(model);
    
    cplex.setParam(IloCplex::TiLim, 3600);
    cplex.setParam(IloCplex::Threads, 4);
    
    int subgradient_iteration {0};
    int rounds_without_improvement {0};
    double theta = 2.0;
    
    std::ofstream results_file;
    std::string results_file_name {"./subgradient_results/" + instance_name + ".txt"};
    results_file.open(results_file_name, std::ios::out);
    
    std::cout << "Logging to " << results_file_name << std::endl;
    
    results_file << " ITER";
    results_file << "      TIME";
    results_file << "       BOUND";
    results_file << "            GAP";
    results_file << "                MTZ";
    results_file << "                PRC";
    results_file << "     THETA";
    results_file << "       STEP LD";
    results_file << "       STEP MU";
    results_file << "      AVG LD";
    results_file << "      AVG MU";
    results_file << "       AVG L";
    results_file << "       AVG M";
    results_file << "  IMPROVED";
    results_file << std::endl;
    
    results_file << "----*";
    results_file << "---------*";
    results_file << "-----------*";
    results_file << "--------------*";
    results_file << "------------------*";
    results_file << "------------------*";
    results_file << "---------*";
    results_file << "-------------*";
    results_file << "-------------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "---------*";
    results_file << std::endl;
    
    results_file.precision(6);
    
    while(subgradient_iteration++ <= iteration_limit) {
        high_resolution_clock::time_point t_start {high_resolution_clock::now()};
        
        if(cplex.solve()) {
            high_resolution_clock::time_point t_end {high_resolution_clock::now()};
            duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};
            
            double obj_const_term {0};
            for(int i = 0; i <= 2*n + 1; i++) {
                for(int j = 0; j <= 2*n + 1; j++) {
                    if(c[i][j] >= 0) { obj_const_term -= lambda[i][j]; }
                }
                if(i >= 1 && i <= n) { obj_const_term -= mu[i]; }
            }

            double iteration_time {time_span.count()};
            double result {cplex.getObjValue() + obj_const_term};
            int violated_mtz {0};
            int violated_prec {0};
            int loose_mtz {0};
            int loose_prec {0};
            int tight_mtz {0};
            int tight_prec {0};
            bool improved {std::abs(best_sol - result) < std::abs(best_sol - best_bound)};
            
            if(improved) {
                best_bound = result;
                best_lambda = lambda;
                best_mu = mu;
                rounds_without_improvement = 0;
            } else {
                rounds_without_improvement++;
            }
            
            IloNumArray xx(env); cplex.getValues(xx, variables_x);
            IloNumArray tt(env); cplex.getValues(tt, variables_tt);
            std::vector<std::vector<double>> x(2*n + 2, std::vector<double>(2*n + 2, 0));
            std::vector<double> t(2*n + 2, 0);
            std::vector<std::vector<double>> L(2*n + 2, std::vector<double>(2*n + 2, 0));
            std::vector<double> M(n, 0);
            double LL {0};
            double MM {0};
            double avg_lambda_before {0};
            double avg_mu_before {0};
            double avg_m {0};
            double avg_l {0};
            int col_n {0};
            
            for(int i = 0; i <= 2*n + 1; i++) {
                t[i] = tt[i];
                if(i >= 1 && i <= n) {
                    M[i] = t[i] + 1 - t[n+i];
                    MM += pow(M[i], 2);
                    avg_m += M[i];
                    avg_mu_before += mu[i];
                }
                for(int j = 0; j <= 2*n + 1; j++) {
                    if(c[i][j] >= 0) {
                        x[i][j] = xx[col_n++];
                        L[i][j] = t[i] + 1 - (2*n + 1) * x[i][j] - t[j];
                        LL += pow(L[i][j], 2);
                        avg_l += L[i][j];
                        avg_lambda_before += lambda[i][j];
                    }
                }
            }
            
            xx.end(); tt.end();
            
            if(rounds_without_improvement > 0 && rounds_without_improvement % 5 == 0) {
                theta /= 2;
            }
            
            double step_lambda {theta * (best_sol - result) / LL};
            double step_mu {theta * (best_sol - result) / MM};
            
            bool exit_condition {true};
            
            for(int i = 0; i <= 2*n + 1; i++) {
                for(int j = 0; j <= 2*n + 1; j++) {
                    if(c[i][j] >= 0) {
                        if(L[i][j] > 0.0 && !floating_equal(L[i][j], 0.0)) { violated_mtz++; }
                        else if(L[i][j] < 0.0 && !floating_equal(L[i][j], 0.0)) { loose_mtz++; }
                        else if(floating_equal(L[i][j], 0.0)) { tight_mtz++; }
                        else { throw std::runtime_error("Floating point comparison screwed up for L"); }
                        lambda[i][j] = std::max(0.0, lambda[i][j] - step_lambda * L[i][j]);
                        exit_condition = exit_condition && (floating_equal(L[i][j], 0.0) || (L[i][j] < 0.0 && floating_equal(lambda[i][j], 0.0)));
                    }
                }
                if(i >= 1 && i <= n) {
                    if(M[i] > 0.0 && !floating_equal(M[i], 0.0)) { violated_prec++; }
                    else if(M[i] < 0.0 && !floating_equal(M[i], 0.0)) { loose_prec++; }
                    else if(floating_equal(M[i], 0.0)) { tight_prec++; }
                    else { throw std::runtime_error("Floating point comparison screwed up for M"); }
                    mu[i] = std::max(0.0, mu[i] - step_mu * M[i]);
                    exit_condition = exit_condition && (floating_equal(M[i], 0.0) || (M[i] < 0 && floating_equal(mu[i], 0.0)));
                }
            }
            
            avg_lambda_before = avg_lambda_before / col_n;
            avg_mu_before = avg_mu_before / n;
            avg_l = avg_l / col_n;
            avg_m = avg_m / n;
            
            results_file << std::setw(5) << subgradient_iteration;
            results_file << std::setw(10) << iteration_time;
            results_file << std::setw(12) << result;
            results_file << std::setw(14) << ((best_sol - result) / best_sol) * 100 << "%";
            results_file << std::setw(5) << violated_mtz << "  " << std::setw(5) << loose_mtz << "  " << std::setw(5) << tight_mtz;
            results_file << std::setw(5) << violated_prec << "  " << std::setw(5) << loose_prec << "  " << std::setw(5) << tight_prec;
            results_file << std::setw(10) << theta;
            results_file << std::setw(14) << step_lambda;
            results_file << std::setw(14) << step_mu;
            results_file << std::setw(12) << avg_lambda_before;
            results_file << std::setw(12) << avg_mu_before;
            results_file << std::setw(12) << avg_l;
            results_file << std::setw(12) << avg_m;
            results_file << std::setw(10) << std::boolalpha << improved << std::endl;
            
            if(exit_condition) {
                results_file << std::endl << "Proven optimal! (by exit condition)" << std::endl;
                results_file << "UB: " << std::setprecision(12) << best_sol << std::endl;
                results_file << "LB: " << std::setprecision(12) << best_bound << std::endl;
                results_file << "|UB - LB|: " << std::setprecision(12) << std::abs(best_sol - best_bound) << std::endl;
                break;
            } else if(floating_equal(best_sol, best_bound)) {
                results_file << std::endl << "Likely optimal! (by small gap)" << std::endl;
                results_file << "UB: " << std::setprecision(12) << best_sol << std::endl;
                results_file << "LB: " << std::setprecision(12) << best_bound << std::endl;
                results_file << "|UB - LB|: " << std::setprecision(12) << std::abs(best_sol - best_bound) << std::endl;
                break;
            } else if(theta < 1 / pow(2, 10)) {
                results_file << std::endl << "Theta too small, the method is not converging or, at least, not fast enough!" << std::endl;
                results_file << "UB: " << std::setprecision(12) << best_sol << std::endl;
                results_file << "LB: " << std::setprecision(12) << best_bound << std::endl;
                results_file << "|UB - LB|: " << std::setprecision(12) << std::abs(best_sol - best_bound) << std::endl;
                break;
            } else {
                int num_col {0};
                for(int i = 0; i <= 2*n + 1; i++) {
                    double t_obj_coeff {0};
                    for(int j = 0; j <= 2*n + 1; j++) {
                        if(c[i][j] >= 0) {
                            double x_obj_coeff {0};
                            
                            x_obj_coeff += c[i][j];
                            x_obj_coeff += (2*n + 1) * lambda[i][j];

                            t_obj_coeff -= lambda[i][j];
                            t_obj_coeff += lambda[j][i];
                            
                            obj.setLinearCoef(variables_x[num_col++], x_obj_coeff);
                        }
                    }

                    if(i >= 1 && i <= n) {
                        t_obj_coeff -= mu[i];
                        t_obj_coeff += mu[n+i];
                    }
                    
                    obj.setLinearCoef(variables_tt[i], t_obj_coeff);
                }
            }
        } else {
            throw std::runtime_error("CPLEX failed!");
        }
    }
    
    results_file.close();
    env.end();
}