#include <global.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

#include <boost/container/vector.hpp>
#include <boost/range/algorithm/find.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>

std::vector<IloRange> SubtourEliminationCutsSolver::separate_valid_cuts(std::shared_ptr<const Graph> g, CallbacksHelper::solution sol, IloNumVarArray x, double eps) {
    using namespace std::chrono;
    
    int tot_number_of_iterations {25};
    int tabu_duration {10};
    int n {g->g[graph_bundle].n};
    auto xvals = sol.x;
    
    boost::container::vector<bool> in_S_pi = boost::container::vector<bool>(2*n+1, false); // Add spaces for 0, even ifwe don't use it
    boost::container::vector<bool> in_S_sigma = boost::container::vector<bool>(2*n+1, false);
    boost::container::vector<bool> in_tabu_pi = boost::container::vector<bool>(2*n+1, false);
    boost::container::vector<bool> in_tabu_sigma = boost::container::vector<bool>(2*n+1, false);
    std::vector<int> tabu_pi_start = std::vector<int>(2*n+1, -1);
    std::vector<int> tabu_sigma_start = std::vector<int>(2*n+1, -1);
    
    boost::container::vector<bool> in_first_set_pi = boost::container::vector<bool>(2*n+1, false); // [i] == true if i \in S \cap \pi(S)
    boost::container::vector<bool> in_second_set_pi = boost::container::vector<bool>(2*n+1, false); // [i] == true if i \in \bar{S} \cap \pi(S)
    boost::container::vector<bool> in_third_set_pi = boost::container::vector<bool>(2*n+1, true); // [i] == true if i \in \bar{S} \setminus \pi(S)
    boost::container::vector<bool> in_first_set_sigma = boost::container::vector<bool>(2*n+1, true);
    boost::container::vector<bool> in_second_set_sigma = boost::container::vector<bool>(2*n+1, false);
    boost::container::vector<bool> in_third_set_sigma = boost::container::vector<bool>(2*n+1, false);
    
    double lhs_pi {0.0};
    double lhs_sigma {0.0};
    
    double first_sum_pi {0.0};
    double second_sum_pi {0.0};
    double third_sum_pi {0.0};
    double first_sum_sigma {0.0};
    double second_sum_sigma {0.0};
    double third_sum_sigma {0.0};
    
    in_tabu_pi[0] = true;
    in_tabu_sigma[0] = true;
    in_third_set_pi[0] = false;
    in_first_set_sigma[0] = true;
    
    std::vector<IloRange> cuts {std::vector<IloRange>()};
    
    high_resolution_clock::time_point t_start {high_resolution_clock::now()};
    
    // std::cerr << "Subtour-elimination cuts solver" << std::endl;
    
    for(int iter = 1; iter <= tot_number_of_iterations; iter++) {
        double best_lhs_pi {lhs_pi};
        double best_lhs_sigma {lhs_sigma};
    
        double best_first_sum_pi {0};
        double best_second_sum_pi {0};
        double best_third_sum_pi {0};
        double best_first_sum_sigma {0};
        double best_second_sum_sigma {0};
        double best_third_sum_sigma {0};
        
        boost::container::vector<bool> best_in_first_set_pi = in_first_set_pi;
        boost::container::vector<bool> best_in_second_set_pi = in_second_set_pi;
        boost::container::vector<bool> best_in_third_set_pi = in_third_set_pi;
        boost::container::vector<bool> best_in_first_set_sigma = in_first_set_sigma;
        boost::container::vector<bool> best_in_second_set_sigma = in_second_set_sigma;
        boost::container::vector<bool> best_in_third_set_sigma = in_third_set_sigma;
        
        int best_node_pi {-1};
        int best_node_sigma {-1};
        
        // std::cerr << "\tIteration: " << iter << std::endl;
        
        for(int i = 1; i <= 2*n; i++) { // Element to add/remove to/from S_pi and S_sigma
            double new_lhs_pi {lhs_pi};
            double new_lhs_sigma {lhs_sigma};
            
            double new_first_sum_pi {0};
            double new_second_sum_pi {0};
            double new_third_sum_pi {0};
            double new_first_sum_sigma {0};
            double new_second_sum_sigma {0};
            double new_third_sum_sigma {0};
            
            boost::container::vector<bool> new_in_first_set_pi = in_first_set_pi;
            boost::container::vector<bool> new_in_second_set_pi = in_second_set_pi;
            boost::container::vector<bool> new_in_third_set_pi = in_third_set_pi;
            boost::container::vector<bool> new_in_first_set_sigma = in_first_set_sigma;
            boost::container::vector<bool> new_in_second_set_sigma = in_second_set_sigma;
            boost::container::vector<bool> new_in_third_set_sigma = in_third_set_sigma;
            
            // std::cerr << "\t\ti: " << i << std::endl;
            
            if(in_S_pi[i]) { // Remove from S_pi
                if(i <= n) {
                    if(in_S_pi[i+n]) {
                        new_in_first_set_pi[i] = false;
                        new_in_second_set_pi[i] = true;
                    } else {
                        new_in_third_set_pi[i] = true;
                    }
                } else {
                    if(in_S_pi[i-n]) {
                        new_in_first_set_pi[i-n] = false;
                        new_in_third_set_pi[i] = true;
                    } else {
                        new_in_second_set_pi[i-n] = false;
                        new_in_third_set_pi[i-n] = true;
                        new_in_third_set_pi[i] = true;
                    }
                }
            } else { // Add to S
                if(!in_tabu_pi[i]) {
                    if(i <= n) {
                        if(in_S_pi[i+n]) {
                            new_in_first_set_pi[i] = true;
                            new_in_second_set_pi[i] = false;
                        } else {
                            new_in_third_set_pi[i] = false;
                        }
                    } else {
                        if(in_S_pi[i-n]) {
                            new_in_first_set_pi[i-n] = true;
                            new_in_third_set_pi[i] = false;
                        } else {
                            new_in_second_set_pi[i-n] = true;
                            new_in_third_set_pi[i-n] = false;
                            new_in_third_set_pi[i] = false;
                        }
                    }
                }
            }
            
            if(in_S_sigma[i]) { // Remove from S_sigma
                if(i <= n) {
                    if(in_S_sigma[i+n]) {
                        new_in_first_set_sigma[i] = true;
                        new_in_second_set_sigma[i+n] = false;
                    } else {
                        new_in_first_set_sigma[i] = true;
                        new_in_first_set_sigma[i+n] = true;
                        new_in_third_set_sigma[i+n] = false;
                    }
                } else {
                    if(in_S_sigma[i-n]) {
                        new_in_second_set_sigma[i] = false;
                        new_in_third_set_sigma[i] = true;
                    } else {
                        new_in_first_set_sigma[i] = true;
                    }
                }
            } else { // Add to S_sigma
                if(!in_tabu_sigma[i]) {
                    if(i <= n) {
                        if(in_S_sigma[i+n]) {
                            new_in_first_set_sigma[i] = false;
                            new_in_second_set_sigma[i+n] = true;
                        } else {
                            new_in_first_set_sigma[i    ] = false;
                            new_in_first_set_sigma[i+n] = false;
                            new_in_third_set_sigma[i+n] = true;
                        }
                    } else {
                        if(in_S_sigma[i-n]) {
                            new_in_second_set_sigma[i] = true;
                            new_in_third_set_sigma[i] = false;
                        } else {
                            new_in_first_set_sigma[i] = false;
                        }
                    }
                }
            }
            
            auto new_in_S_pi = in_S_pi;
            new_in_S_pi[i] = !in_S_pi[i];
            
            auto new_in_S_sigma = in_S_sigma;
            new_in_S_sigma[i] = !in_S_sigma[i];
            
            for(int ii = 0; ii <= 2*n+1; ii++) {
                for(int jj = 0; jj <= 2*n+1; jj++) {
                    if(g->cost[ii][jj] >= 0) {
                        // PI
                        
                        // First sum: delta+(S') \cup delta-(S')
                        if((ii >= 1 && ii <= 2*n && new_in_S_pi[ii]) || (jj >= 1 && jj <= 2*n && new_in_S_pi[jj])) {
                            new_first_sum_pi += xvals[ii][jj];
                        }
                        if(ii >= 1 && ii <= 2*n && jj >= 1 && jj <= 2*n) {
                            // Second sum: first set -> third set
                            if(new_in_first_set_pi[ii] && new_in_third_set_pi[jj]) {
                                new_second_sum_pi += xvals[ii][jj];
                            }
                            // Third sum: S' -> second set
                            if(new_in_S_pi[ii] && new_in_second_set_pi[jj]) {
                                new_third_sum_pi += xvals[ii][jj];
                            }
                        }
                        
                        // SIGMA
                        
                        // First sum: delta+(S') \cup delta-(S')
                        if((ii >= 1 && ii <= 2*n && new_in_S_pi[ii]) || (jj >= 1 && jj <= 2*n && new_in_S_pi[jj])) {
                            new_first_sum_sigma += xvals[ii][jj];
                        }
                        if(ii >= 1 && ii <= 2*n && jj >= 1 && jj <= 2*n) {
                            // Second sum: first set -> second set
                            if(new_in_first_set_sigma[ii] && new_in_second_set_sigma[jj]) {
                                new_second_sum_sigma += xvals[ii][jj];
                            }
                            // Third sum: third set -> S'
                            if(new_in_third_set_sigma[ii] && new_in_S_sigma[ii]) {
                                new_third_sum_sigma += xvals[ii][jj];
                            }
                        }
                    }
                }
            }
            
            new_lhs_pi = new_first_sum_pi - 2 * new_second_sum_pi - 2 * new_third_sum_pi;
            
            // std::cerr << "\t\t\tLHS pi: " << new_first_sum_pi << " - 2 * " << new_second_sum_pi << " - 2 * " << new_third_sum_pi << " = " << new_lhs_pi << std::endl;
            
            int first_non_tabu_pi = boost::find(in_tabu_pi, false) - boost::begin(in_tabu_pi);
            
            if(i == first_non_tabu_pi || (new_lhs_pi < lhs_pi && !in_tabu_pi[i])) {
                best_lhs_pi = new_lhs_pi;
                best_first_sum_pi = new_first_sum_pi;
                best_second_sum_pi = new_second_sum_pi;
                best_third_sum_pi = new_third_sum_pi;
                best_node_pi = i;
                best_in_first_set_pi = new_in_first_set_pi;
                best_in_second_set_pi = new_in_second_set_pi;
                best_in_third_set_pi = new_in_third_set_pi;
            }
            
            new_lhs_sigma = new_first_sum_sigma - 2 * new_second_sum_sigma - 2 * new_third_sum_sigma;
            
            // std::cerr << "\t\t\tLHS sigma: " << new_first_sum_sigma << " - 2 * " << new_second_sum_sigma << " - 2 * " << new_third_sum_sigma << " = " << new_lhs_sigma << std::endl;
            
            int first_non_tabu_sigma = boost::find(in_tabu_sigma, false) - boost::begin(in_tabu_sigma);
            
            if(i == first_non_tabu_sigma || (new_lhs_sigma < lhs_sigma && !in_tabu_sigma[i])) {
                best_lhs_sigma = new_lhs_sigma;
                best_first_sum_sigma = new_first_sum_sigma;
                best_second_sum_sigma = new_second_sum_sigma;
                best_third_sum_sigma = new_third_sum_sigma;
                best_node_sigma = i;
                best_in_first_set_sigma = new_in_first_set_sigma;
                best_in_second_set_sigma = new_in_second_set_sigma;
                best_in_third_set_sigma = new_in_third_set_sigma;
            }
        }
        
        if(best_node_pi == -1) {
            throw std::runtime_error("best_node_pi == -1 should not happen!");
        } else {
            bool removed {in_S_pi[best_node_pi]};
            
            // std::cerr << "\t\tBest node (pi): " << best_node_pi << " " << (removed ? "deletion" : "insertion") << ", lhs: " << best_lhs_pi << std::endl;
            
            in_S_pi[best_node_pi] = !in_S_pi[best_node_pi];
            if(removed) {
                in_tabu_pi[best_node_pi] = true;
                tabu_pi_start[best_node_pi] = iter;
            }
            in_first_set_pi = best_in_first_set_pi;
            in_second_set_pi = best_in_second_set_pi;
            in_third_set_pi = best_in_third_set_pi;
            lhs_pi = best_lhs_pi;
            in_first_set_pi = best_in_first_set_pi;
            in_second_set_pi = best_in_second_set_pi;
            in_third_set_pi = best_in_third_set_pi;
            
            // Update tabu list
            for(int ii = 1; ii <= 2*n; ii++) {
                if(tabu_pi_start[ii] == iter - tabu_duration) {
                    tabu_pi_start[ii] = -1;
                    in_tabu_pi[ii] = false;
                }
            }
            
            // std::cerr << "\t\tTabu list (pi): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_tabu_pi[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tS (pi): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_S_pi[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tS cap pi(S) (pi): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_first_set_pi[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tbar S cap pi(S) (pi): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_second_set_pi[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tbar S minus pi(S) (pi): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_third_set_pi[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            
            // Add cut if violated
            if(lhs_pi < 2 - eps && boost::find(in_S_pi, true) != boost::end(in_S_pi)) {
                IloExpr lhs;
                IloNum rhs = 2;
                bool expr_init {false};
                
                // std::cerr << "\t\tCut (pi): ";
                
                int col_index {0};
                for(int i = 0; i <= 2 * n + 1; i++) {
                    for(int j = 0; j <= 2 * n + 1; j++) {
                        if(g->cost[i][j] >= 0) {
                            if(i >= 1 && i <= 2*n && j >= 1 && j <= 2*n) {
                                if(in_S_pi[i]) { // a \in \delta^+(S)
                                    // std::cerr << "+ x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = x[col_index]; expr_init = true; } else { lhs += x[col_index]; }
                                }
                                if(in_S_pi[j]) { // a \in \delta^-(S)
                                    // std::cerr << "+ x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = x[col_index]; expr_init = true; } else { lhs += x[col_index]; }
                                }
                                if(in_first_set_pi[i] && in_third_set_pi[j]) { // a \in second sum
                                    // std::cerr << "-2 x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = -2 * x[col_index]; expr_init = true; } else { lhs += -2 * x[col_index]; }
                                }
                                if(in_S_pi[i] && in_second_set_pi[j]) { // a \in third sum
                                    // std::cerr << "-2 x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = -2 * x[col_index]; expr_init = true; } else { lhs += -2 * x[col_index]; }
                                }
                            }
                            col_index++;
                        }
                    }
                }
                
                // std::cerr << std::endl;
                
                IloRange cut;
                cut = (lhs >= rhs);
                cuts.push_back(cut);
            }
        }
        
        if(best_node_sigma == -1) {
            throw std::runtime_error("best_node_sigma == -1 should not happen!");
        } else {
            bool removed = in_S_sigma[best_node_sigma];
            
            // std::cerr << "\t\tBest node (sigma): " << best_node_sigma << " " << (removed ? "deletion" : "insertion") << ", lhs: " << best_lhs_sigma << std::endl;
            
            in_S_sigma[best_node_sigma] = !in_S_sigma[best_node_sigma];
            if(removed) {
                in_tabu_sigma[best_node_sigma] = true;
                tabu_sigma_start[best_node_sigma] = iter;
            }
            in_first_set_sigma = best_in_first_set_sigma;
            in_second_set_sigma = best_in_second_set_sigma;
            in_third_set_sigma = best_in_third_set_sigma;
            lhs_sigma = best_lhs_sigma;
            in_first_set_sigma = best_in_first_set_sigma;
            in_second_set_sigma = best_in_second_set_sigma;
            in_third_set_sigma = best_in_third_set_sigma;
            
            // Update tabu list
            for(int ii = 1; ii <= 2*n; ii++) {
                if(tabu_sigma_start[ii] == iter - tabu_duration) {
                    tabu_sigma_start[ii] = -1;
                    in_tabu_sigma[ii] = false;
                }
            }
            
            // std::cerr << "\t\tTabu list (sigma): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_tabu_sigma[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tS (sigma): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_S_sigma[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tbar S minus sigma(S) (sigma): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_first_set_sigma[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tS cap sigma(S) (sigma): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_second_set_sigma[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            //
            // std::cerr << "\t\tbar S cap sigma(S) (sigma): ";
            // for(int ii = 1; ii <= 2*n; ii++) {
            //     if(in_third_set_sigma[ii]) {
            //         std::cerr << ii << " ";
            //     }
            // }
            // std::cerr << std::endl;
            
            // Add cut if violated
            if(lhs_sigma < 2 - eps && boost::find(in_S_sigma, true) != boost::end(in_S_sigma)) {
                IloExpr lhs;
                IloNum rhs = 2;
                bool expr_init {false};
                
                // std::cerr << "\t\tCut (sigma): ";
                
                int col_index {0};
                for(int i = 0; i <= 2 * n + 1; i++) {
                    for(int j = 0; j <= 2 * n + 1; j++) {
                        if(g->cost[i][j] >= 0) {
                            if(i >= 1 && i <= 2*n && j >= 1 && j <= 2*n) {
                                if(in_S_sigma[i]) { // a \in \delta^+(S)
                                    // std::cerr << "+ x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = x[col_index]; expr_init = true; } else { lhs += x[col_index]; }
                                }
                                if(in_S_sigma[j]) { // a \in \delta^-(S)
                                    // std::cerr << "+ x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = x[col_index]; expr_init = true; } else { lhs += x[col_index]; }
                                }
                                if(in_first_set_sigma[i] && in_second_set_sigma[j]) { // a \in second sum
                                    // std::cerr << "-2 x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = -2 * x[col_index]; expr_init = true; } else { lhs += -2 * x[col_index]; }
                                }
                                if(in_third_set_sigma[i] && in_S_sigma[j]) { // a \in third sum
                                    // std::cerr << "-2 x[" << i << "][" << j << "] ";
                                    if(!expr_init) { lhs = -2 * x[col_index]; expr_init = true; } else { lhs += -2 * x[col_index]; }
                                }
                            }
                            col_index++;
                        }
                    }
                }
                
                // std::cerr << std::endl;
                
                IloRange cut;
                cut = (lhs >= rhs);
                cuts.push_back(cut);
            }
        }
    }
    
    high_resolution_clock::time_point t_end {high_resolution_clock::now()};
    duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};
    global::g_total_time_spent_separating_cuts += time_span.count();
    
    return cuts;
}