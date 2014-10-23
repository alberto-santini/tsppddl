#include <solver/subgradient/subgradient_solver.h>

#include <ilcplex/ilocplex.h>

// #include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

SubgradientSolver::SubgradientSolver(std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, std::string instance_path, int iteration_limit) : g{g}, initial_solutions{initial_solutions}, iteration_limit{iteration_limit}, best_sol{std::numeric_limits<double>::max()} {
    // PORTABLE WAY:
    // boost::filesystem::path i_path(instance_path);
    // std::stringstream ss; ss << i_path.stem();
    // instance_name = ss.str();
    
    // NOT-PORTABLE WAY:
    std::vector<std::string> path_parts;
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    std::vector<std::string> file_parts;
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));
    file_parts.pop_back();
    
    instance_name = boost::algorithm::join(file_parts, ".");
}

void SubgradientSolver::solve(bool lg_mtz, bool lg_prec) {
    using namespace std::chrono;
    
    int n {g->g[graph_bundle].n};
    int n_arcs {(int)num_edges(g->g)};
    int Q {g->g[graph_bundle].capacity};
    demand_t d {g->demand};
    draught_t l {g->draught};
    cost_t c {g->cost};
    
    auto best_heur_path = std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
    best_sol = best_heur_path->total_cost;
    
    // Initial multipliers: all 1.0
    std::vector<std::vector<double>> lambda(2 * n + 2, std::vector<double>(2 * n + 2, 1.0));
    std::vector<double> mu(n+1, 1.0); // I include mu[0] for convenience
    
    double best_bound {0};
    std::vector<std::vector<double>> best_lambda(2 * n + 2, std::vector<double>(2 * n + 2, 1.0));
    std::vector<double> best_mu(n+1, 1.0);
    
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
    
    IloRangeArray mtz(env);
    IloRangeArray prec(env);
    
    IloObjective obj = IloMinimize(env);
    
    #include <solver/subgradient/lagrange_setup_model.raw.cpp>
    
    model.add(obj);
    model.add(variables_x); model.add(variables_y); model.add(variables_tt);
    model.add(outdegree); model.add(indegree); model.add(load); model.add(y_lower); model.add(y_upper); model.add(initial_load); model.add(initial_order);
    if(!lg_mtz) {
        model.add(mtz);
    }
    if(!lg_prec) {
        model.add(prec);
    }
    
    IloCplex cplex(model);
    
    cplex.setParam(IloCplex::TiLim, 3600);
    cplex.setParam(IloCplex::Threads, 4);
    
    int subgradient_iteration {0};
    int rounds_without_improvement {0};
    double theta = 2.0;
    
    std::ofstream results_file;
    results_file.open("./subgradient_results/results/" + instance_name + (lg_mtz ? "_mtz" : "") + (lg_prec ? "_prec" : "") + ".txt", std::ios::out);
    print_headers(results_file);
	
	std::ofstream mult_dump;
	if(n < 3) { mult_dump.open("./subgradient_results/mult_dump_" + instance_name + (lg_mtz ? "_mtz" : "") + (lg_prec ? "_prec" : "") + ".txt", std::ios::out); }
    
    cplex.exportModel("./subgradient_results/model.lp");
    
    while(subgradient_iteration++ <= iteration_limit) {
        high_resolution_clock::time_point t_start {high_resolution_clock::now()};
        
        if(cplex.solve()) {
            high_resolution_clock::time_point t_end {high_resolution_clock::now()};
            duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};
            
            double obj_const_term {0};
            for(int i = 0; i <= 2*n + 1; i++) {
                if(lg_mtz) { 
                    for(int j = 0; j <= 2*n + 1; j++) {
                        if(c[i][j] >= 0) { obj_const_term -= (2*n + 1) * lambda[i][j]; }
                    }
                }
                if(lg_prec) {
                    if(i >= 1 && i <= n) { obj_const_term += mu[i]; }
                }
            }
            
            double iteration_time {time_span.count()};
            double result {cplex.getObjValue() + obj_const_term};
            int violated_mtz {0};
            int violated_prec {0};
            int loose_mtz {0};
            int loose_prec {0};
            int tight_mtz {0};
            int tight_prec {0};
            bool improved {best_sol - result < best_sol - best_bound};
            
            if(improved) {
                best_bound = result;
                if(lg_mtz) { best_lambda = lambda; }
                if(lg_prec) { best_mu = mu; }
                rounds_without_improvement = 0;
            } else {
                rounds_without_improvement++;
            }
            
            IloNumArray x(env); cplex.getValues(x, variables_x);
            IloNumArray t(env); cplex.getValues(t, variables_tt);
                        
            // Value of the relaxed inequalities, as put in the objective function
            std::vector<std::vector<double>> L(2*n + 2, std::vector<double>(2*n + 2, 0));
            std::vector<double> M(n+1, 0);
            
            // Sum of the squares of L and M
            double LL {0};
            double MM {0};
            
            // Average multipliers, before adjustment
            double avg_lambda_before {0};
            double avg_mu_before {0};
            
            // Aferage of L and M
            double avg_l {0};
            double avg_m {0};
            
            int col_n {0};
            for(int i = 0; i <= 2*n + 1; i++) {
                if(lg_mtz) {
                    for(int j = 0; j <= 2*n + 1; j++) {
                        if(c[i][j] >= 0) {
                            L[i][j] = t[i] + 1 - (2*n + 1) * (1 - x[col_n++]) - t[j];
                            LL += pow(L[i][j], 2); avg_l += L[i][j]; avg_lambda_before += lambda[i][j];
                        }
                    }
                }
                if(lg_prec) {
                    if(i >= 1 && i <= n) {
                        M[i] = t[i] + 1 - t[n+i];
                        MM += pow(M[i], 2); avg_m += M[i]; avg_mu_before += mu[i];
                    }
                }
            }
                        
            x.end(); t.end();
                        
            // Halve theta after 5 rounds without improvement
            if(rounds_without_improvement > 0 && rounds_without_improvement % 5 == 0) {
                theta /= 2;
            }
                        
            double step_lambda {0}; if(lg_mtz) { step_lambda = theta * (best_sol - result) / LL; }
            double step_mu {0}; if(lg_prec) { step_mu = theta * (best_sol - result) / MM; }
            
            bool exit_condition {true};
            
            for(int i = 0; i <= 2*n + 1; i++) {
                if(lg_mtz) {
                    for(int j = 0; j <= 2*n + 1; j++) {
                        if(c[i][j] >= 0) {
                            if(L[i][j] > 0.0 && !sg_compare::floating_equal(L[i][j], 0.0)) { violated_mtz++; }
                            else if(L[i][j] < 0.0 && !sg_compare::floating_equal(L[i][j], 0.0)) { loose_mtz++; }
                            else if(sg_compare::floating_equal(L[i][j], 0.0)) { tight_mtz++; }

                            // Update multiplier
                            lambda[i][j] = std::max(0.0, lambda[i][j] + step_lambda * L[i][j]);
                        
                            // Exit condition, by complementary slackness
                            exit_condition = exit_condition && (sg_compare::floating_equal(L[i][j], 0.0) || (L[i][j] < 0.0 && sg_compare::floating_equal(lambda[i][j], 0.0)));
                        }
                    }
                }
                if(lg_prec) {
                    if(i >= 1 && i <= n) {
                        if(M[i] > 0.0 && !sg_compare::floating_equal(M[i], 0.0)) { violated_prec++; }
                        else if(M[i] < 0.0 && !sg_compare::floating_equal(M[i], 0.0)) { loose_prec++; }
                        else if(sg_compare::floating_equal(M[i], 0.0)) { tight_prec++; }

                        // Update multiplier
                        mu[i] = std::max(0.0, mu[i] + step_mu * M[i]);
                    
                        // Exit condition, by complementary slackness
                        exit_condition = exit_condition && (sg_compare::floating_equal(M[i], 0.0) || (M[i] < 0 && sg_compare::floating_equal(mu[i], 0.0)));
                    }
                }
            }
                        
            if(lg_mtz) { avg_lambda_before = avg_lambda_before / n_arcs; avg_l = avg_l / n_arcs; }
            if(lg_prec) { avg_mu_before = avg_mu_before / n; avg_m = avg_m / n; }
                        
            print_result_row(results_file, result, best_sol, subgradient_iteration, iteration_time, cplex.getObjValue(), obj_const_term, violated_mtz, loose_mtz, tight_mtz, violated_prec, loose_prec, tight_prec, theta, step_lambda, step_mu, avg_lambda_before, avg_mu_before, avg_l, avg_m, improved, lg_mtz, lg_prec);
			if(n < 3) { print_mult_dump(mult_dump, L, lambda); }

            if(exit_condition) {
                results_file << std::endl << "Proven optimal! (by exit condition)" << std::endl;
                print_final_results(results_file, best_sol, best_bound);
                break;
            } else if(sg_compare::floating_optimal(best_sol, best_bound)) {
                results_file << std::endl << "Likely optimal! (by small gap)" << std::endl;
                print_final_results(results_file, best_sol, best_bound);
                break;
            } else if(theta < 1 / pow(2, 10)) {
                results_file << std::endl << "Theta too small, the method is not converging or, at least, not fast enough!" << std::endl;
                print_final_results(results_file, best_sol, best_bound);
                break;
            } else {                
                // Update objective function coefficients
                int num_col {0};
                for(int i = 0; i <= 2*n + 1; i++) {
                    double t_obj_coeff {0};
                    for(int j = 0; j <= 2*n + 1; j++) {
                        if(c[i][j] >= 0) {
                            double x_obj_coeff {0};
                            
                            x_obj_coeff += c[i][j];
                            
                            if(lg_mtz) {
                                x_obj_coeff += (2*n + 1) * lambda[i][j];
                                t_obj_coeff += lambda[i][j];
                            }
                            
                            obj.setLinearCoef(variables_x[num_col++], x_obj_coeff);
                        }
                        
                        if(lg_mtz) {
                            if(c[j][i] >= 0) {
                                t_obj_coeff -= lambda[j][i];
                            }
                        }
                    }

                    if(lg_prec) {
                        if(i >= 1 && i <= n) {
                            t_obj_coeff += mu[i];
                        } else if(i >= n+1 && i <= 2*n) {
                            t_obj_coeff -= mu[i-n];
                        }
                    }
                    
                    obj.setLinearCoef(variables_tt[i], t_obj_coeff);
                }
            }
        } else {
            std::cerr << "CPLEX Status: " << cplex.getStatus() << std::endl;
            std::cerr << "CPLEX Inner Status: " << cplex.getCplexStatus() << std::endl;
            std::cerr << "Dumping model on model_err.lp" << std::endl;
            cplex.exportModel("model_err.lp");
            throw std::runtime_error("CPLEX failed!");
        }        
    }
    
    if(subgradient_iteration > iteration_limit) {
        results_file << std::endl << "Iterations limit reached!" << std::endl;
        print_final_results(results_file, best_sol, best_bound);
    }
    
    results_file.close();
    env.end();
}

void SubgradientSolver::print_mult_dump(std::ofstream& dump_file, const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& lambda) const {
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
	
	dump_file << "(i,j)\t";
    for(int i = 0; i <= 2*n + 1; i++) {
        for(int j = 0; j <= 2*n + 1; j++) {
            if(c[i][j] >= 0) {
				dump_file << "(" << std::setw(2) << i << "," << std::setw(2) << j << ")  \t";
			}
		}
	}
	dump_file << std::endl;
	
	dump_file << "L\t";
    for(int i = 0; i <= 2*n + 1; i++) {
        for(int j = 0; j <= 2*n + 1; j++) {
            if(c[i][j] >= 0) {
				dump_file << std::setw(9) << L[i][j] << "\t";
			}
		}
	}
	dump_file << std::endl;

	dump_file << "lbd\t";
    for(int i = 0; i <= 2*n + 1; i++) {
        for(int j = 0; j <= 2*n + 1; j++) {
            if(c[i][j] >= 0) {
				dump_file << std::setw(9) << lambda[i][j] << "\t";
			}
		}
	}
	dump_file << std::endl;
}

void SubgradientSolver::print_headers(std::ofstream& results_file) const {
    results_file << " ITER";
    results_file << "      TIME";
    results_file << "    UB";
    results_file << "          LB";
    results_file << "     = cplex";
    results_file << "     + const";
    results_file << "            GAP";
    results_file << "                MTZ";
    results_file << "                PRC";
    results_file << "       THETA";
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
    results_file << "-----*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "--------------*";
    results_file << "------------------*";
    results_file << "------------------*";
    results_file << "-----------*";
    results_file << "-------------*";
    results_file << "-------------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "-----------*";
    results_file << "---------*";
    results_file << std::endl;
}

void SubgradientSolver::print_result_row(std::ofstream& results_file, double result, double best_sol, int subgradient_iteration, double iteration_time, double cplex_obj, double obj_const_term, int violated_mtz, int loose_mtz, int tight_mtz, int violated_prec, int loose_prec, int tight_prec, double theta, double step_lambda, double step_mu, double avg_lambda_before, double avg_mu_before, double avg_l, double avg_m, bool improved, bool lg_mtz, bool lg_prec) const {
    results_file.precision(6);
    if(result > best_sol) {
        results_file << "!" << std::setw(4) << subgradient_iteration;
    } else {
        results_file << std::setw(5) << subgradient_iteration;
    }
    results_file << std::setw(10) << iteration_time;
    results_file << std::setw(6) << best_sol;
    results_file << std::setw(12) << std::setprecision(8) << result << std::setprecision(6);
    results_file << std::setw(12) << std::setprecision(8) << cplex_obj << std::setprecision(6);
    results_file << std::setw(12) << std::setprecision(8) << obj_const_term << std::setprecision(6);
    results_file << std::setw(14) << ((best_sol - result) / best_sol) * 100 << "%";
    if(lg_mtz) {
        results_file << std::setw(5) << violated_mtz << "  " << std::setw(5) << loose_mtz << "  " << std::setw(5) << tight_mtz;
    } else {
        results_file << std::setw(5) << "-" << "  " << std::setw(5) << "-" << "  " << std::setw(5) << "-";
    }
    if(lg_prec) {
        results_file << std::setw(5) << violated_prec << "  " << std::setw(5) << loose_prec << "  " << std::setw(5) << tight_prec;
    } else {
        results_file << std::setw(5) << "-" << "  " << std::setw(5) << "-" << "  " << std::setw(5) << "-";
    }
    results_file << std::setw(12) << theta;
    if(lg_mtz) {
        results_file << std::setw(14) << step_lambda;
    } else {
        results_file << std::setw(14) << "-";
    }
    if(lg_prec) {
        results_file << std::setw(14) << step_mu;
    } else {
        results_file << std::setw(14) << "-";
    }
    if(lg_mtz) {
        results_file << std::setw(12) << avg_lambda_before;
    } else {
        results_file << std::setw(12) << "-";
    }
    if(lg_prec) {
        results_file << std::setw(12) << avg_mu_before;
    } else {
        results_file << std::setw(12) << "-";
    }
    if(lg_mtz) {
        results_file << std::setw(12) << avg_l;
    } else {
        results_file << std::setw(12) << "-";
    }
    if(lg_prec) {
        results_file << std::setw(12) << avg_m;
    } else {
        results_file << std::setw(12) << "-";
    }
    results_file << std::setw(10) << std::boolalpha << improved << std::endl;
}

void SubgradientSolver::print_final_results(std::ofstream& results_file, double ub, double lb) const {
    results_file << "UB: " << std::setprecision(12) << ub << std::endl;
    results_file << "LB: " << std::setprecision(12) << lb << std::endl;
    results_file << "|UB - LB|: " << std::setprecision(12) << std::abs(ub - lb) << std::endl;
    results_file << "Gap %: " << std::setprecision(12) << (100 * (ub - lb) / ub) << std::endl;
};