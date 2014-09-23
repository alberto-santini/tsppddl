#include <global.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>

SubtourEliminationCutsSolver::SubtourEliminationCutsSolver(std::shared_ptr<const Graph> g, CallbacksHelper::solution sol, IloEnv env, IloNumVarArray x, double eps)
    :   g{g},
        sol{sol},
        env{env},
        x{x},
        eps{eps},
        n{g->g[graph_bundle].n},
        pi{sets_info(
            bvec(2*n+1, false),
            bvec(2*n+1, false),
            ivec(2*n+1, -1),
            bvec(2*n+1, false), 0,
            bvec(2*n+1, false), 0,
            bvec(2*n+1, true), 0, 0)
        },
        sigma{sets_info(
            bvec(2*n+1, false),
            bvec(2*n+1, false),
            ivec(2*n+1, -1),
            bvec(2*n+1, true), 0,
            bvec(2*n+1, false), 0,
            bvec(2*n+1, false), 0, 0)
        },
        tot_number_of_iterations{25},
        tabu_duration{10}
{
    pi.in_ts[0] = false; pi.in_tabu[0] = true;
    sigma.in_fs[0] = false; sigma.in_tabu[0] = true;
}

std::vector<IloRange> SubtourEliminationCutsSolver::separate_valid_cuts() {
    using namespace std::chrono;
    using time_p =  high_resolution_clock::time_point;
    
    auto xvals = sol.x;
    std::vector<IloRange> cuts;
    
    time_p start_time {high_resolution_clock::now()};
    
    for(int iter = 1; iter < tot_number_of_iterations; iter++) {
        sets_info best_pi = pi, best_sigma = sigma;
        int bn_pi = -1, bn_sigma = -1; // Best node \in (1...2n)
        
        for(int i = 1; i <= 2*n; i++) {
            std::cerr << "Iteration " << iter << ", i: " << i << std::endl;
            
            sets_info new_pi = pi, new_sigma = sigma;
            
            std::cerr << "\tInitialised new_pi and new_sigma" << std::endl;
            
            add_or_remove_from_pi_sets(new_pi, i);
            std::cerr << "\tRecomputed sets (pi)" << std::endl;
            recalculate_pi_sums(new_pi, xvals);
            std::cerr << "\tRecomputed sums (pi)" << std::endl;
            add_or_remove_from_sigma_sets(new_sigma, i);
            std::cerr << "\tRecomputed sets (sigma)" << std::endl;
            recalculate_sigma_sums(new_sigma, xvals);
            std::cerr << "\tRecomputed sums (sigma)" << std::endl;
            
            if(i == pi.first_non_tabu() || (new_pi.lhs < best_pi.lhs && !pi.in_tabu[i])) { 
                std::cerr << "\t" << i << " is the new best for pi for this iteration" << std::endl;
                best_pi = new_pi;
                bn_pi = i;
                std::cerr << "\tUpdated best pi" << std::endl;
            } else { std::cerr << "\t" << i << " does not improve the current best for pi" << std::endl; }
            if(i == sigma.first_non_tabu() || (new_sigma.lhs < best_sigma.lhs && !sigma.in_tabu[i])) {
                std::cerr << "\t" << i << " is the new best for sigma for this iteration" << std::endl;
                best_sigma = new_sigma;
                bn_sigma = i;
                std::cerr << "\tUpdated best sigma" << std::endl;
            } else { std::cerr << "\t" << i << " does not improve the current best for sigma" << std::endl; }
        }
        
        std::cerr << "End of iteration " << iter << std::endl;
        
        if(bn_pi == -1) { throw std::runtime_error("Best pi node can't be -1"); } else {
            update_info(pi, best_pi, bn_pi, iter);
            std::cerr << "\tUpdated info for pi" << std::endl;
            add_pi_cut_if_violated(cuts, pi);
            std::cerr << "\tAdded cut for pi" << std::endl;
        }
        if(bn_sigma == -1) { throw std::runtime_error("Best sigma node can't be -1"); } else {
            update_info(sigma, best_sigma, bn_sigma, iter);
            std::cerr << "\tUpdated info for sigma" << std::endl;
            add_sigma_cut_if_violated(cuts, sigma);
            std::cerr << "\tAdded cut for sigma" << std::endl;
        }
    }
    
    time_p end_time {high_resolution_clock::now()};
    duration<double> time_span {duration_cast<duration<double>>(end_time - start_time)};
    global::g_total_time_spent_separating_cuts += time_span.count();
    
    return cuts;
}

void SubtourEliminationCutsSolver::update_info(sets_info& set, sets_info best, int bn, int iter) {
    bool removed {set.in_S[bn]};
    
    set = best;
    if(removed) {
        set.in_tabu[bn] = true;
        set.tabu_start[bn] = iter;
    }
    
    // Update tabu list
    for(int i = 1; i <= 2*n; i++) {
        if(set.tabu_start[i] == iter - tabu_duration) {
            set.in_tabu[i] = false;
            set.tabu_start[i] = -1;
        }
    }
}

void SubtourEliminationCutsSolver::add_pi_cut_if_violated(std::vector<IloRange>& cuts, sets_info pi) {
    if(pi.lhs < 2 && !pi.empty_S()) {
        IloExpr lhs(env);
        IloNum rhs = 2.0;
        
        int col_index {0};
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(g->cost[i][j] >= 0) {
                    if(i >= 1 && i <= 2*n && j >= 1 && j <= 2*n) {
                        if(pi.in_S[i] && !pi.in_S[j]) {
                            lhs += x[col_index];
                        }
                        if(!pi.in_S[i] && pi.in_S[j]) {
                            lhs += x[col_index];
                        }
                        if(pi.in_fs[i] && pi.in_ts[j]) {
                            lhs -= 2 * x[col_index];
                        }
                        if(pi.in_S[i] && pi.in_ss[j]) {
                            lhs -= 2 * x[col_index];
                        }
                    }
                    col_index++;
                }
            }
        }
        
        IloRange cut;
        cut = (lhs >= rhs);
        cuts.push_back(cut);
    }
}

void SubtourEliminationCutsSolver::add_sigma_cut_if_violated(std::vector<IloRange>& cuts, sets_info sigma) {
    if(sigma.lhs < 2 && !sigma.empty_S()) {
        IloExpr lhs(env);
        IloNum rhs = 2.0;
        
        int col_index {0};
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(g->cost[i][j] >= 0) {
                    if(i >= 1 && i <= 2*n && j >= 1 && j <= 2*n) {
                        if(sigma.in_S[i] && !sigma.in_S[j]) {
                            lhs += x[col_index];
                        }
                        if(!pi.in_S[i] && pi.in_S[j]) {
                            lhs += x[col_index];
                        }
                        if(sigma.in_fs[i] && sigma.in_ss[j]) {
                            lhs -= 2 * x[col_index];
                        }
                        if(sigma.in_ts[i] && sigma.in_S[j]) {
                            lhs -= 2 * x[col_index];
                        }
                    }
                    col_index++;
                }
            }
        }
        
        IloRange cut;
        cut = (lhs >= rhs);
        cuts.push_back(cut);
    }
}

void SubtourEliminationCutsSolver::add_or_remove_from_pi_sets(sets_info& pi, int i) {
    if(pi.in_S[i]) { // Remove
        if(i <= n) {
            if(pi.in_S[i+n]) {
                pi.in_fs[i] = false;
                pi.in_ss[i] = true;
            } else {
                pi.in_ts[i] = true;
            }
        } else {
            if(pi.in_S[i-n]) {
                pi.in_fs[i-n] = false;
                pi.in_ts[i] = true;
            } else {
                pi.in_ss[i-n] = false;
                pi.in_ts[i-n] = true;
                pi.in_ts[i] = true;
            }
        }
    } else if(!pi.in_tabu[i]) { // Add
        if(i <= n) {
            if(pi.in_S[i+n]) {
                pi.in_fs[i] = true;
                pi.in_ss[i] = false;
            } else {
                pi.in_ts[i] = false;
            }
        } else {
            if(pi.in_S[i-n]) {
                pi.in_fs[i-n] = true;
                pi.in_ts[i] = false;
            } else {
                pi.in_ss[i-n] = true;
                pi.in_ts[i-n] = false;
                pi.in_ts[i] = false;
            }
        }
    }
    pi.in_S[i] = !pi.in_S[i];
}

void SubtourEliminationCutsSolver::add_or_remove_from_sigma_sets(sets_info& sigma, int i) {
    if(sigma.in_S[i]) { // Remove
        if(i <= n) {
            if(sigma.in_S[i+n]) {
                sigma.in_fs[i] = true;
                sigma.in_ss[i+n] = false;
            } else {
                sigma.in_fs[i] = true;
                sigma.in_fs[i+n] = true;
                sigma.in_ts[i+n] = false;
            }
        } else {
            if(sigma.in_S[i-n]) {
                sigma.in_ss[i] = false;
                sigma.in_ts[i] = true;
            } else {
                sigma.in_fs[i] = true;
            }
        }
    } else if(!sigma.in_tabu[i]) { // Add
        if(i <= n) {
            if(sigma.in_S[i+n]) {
                sigma.in_fs[i] = false;
                sigma.in_ss[i+n] = true;
            } else {
                sigma.in_fs[i] = false;
                sigma.in_fs[i+n] = false;
                sigma.in_ts[i+n] = true;
            }
        } else {
            if(sigma.in_S[i-n]) {
                sigma.in_ss[i] = true;
                sigma.in_ts[i] = false;
            } else {
                sigma.in_fs[i] = false;
            }
        }
    }
    sigma.in_S[i] = !sigma.in_S[i];
}

void SubtourEliminationCutsSolver::recalculate_pi_sums(sets_info& pi, std::vector<std::vector<double>> xvals) {
    pi.fs = 0; pi.ss = 0; pi.ts = 0; pi.lhs = 0;
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(i >= 1 && i <= 2*n && pi.in_S[i] && !pi.in_S[j]) {
                    pi.fs += xvals[i][j];
                }
                if(j >= 1 && j <= 2*n && pi.in_S[j] && !pi.in_S[i]) {
                    pi.fs += xvals[i][j];
                }
                if(i >= 1 && i <= 2*n && pi.in_fs[i] && j >= 1 && j <= 2*n && pi.in_ts[j]) {
                    pi.fs += xvals[i][j];
                }
                if(i >= 1 && i <= 2*n && pi.in_S[i] && j >= 1 && j <= 2*n && pi.in_ss[j]) {
                    pi.fs += xvals[i][j];
                }
            }
        }
    }
    
    pi.lhs = pi.fs - 2 * pi.ss - 2 * pi.ts;
}

void SubtourEliminationCutsSolver::recalculate_sigma_sums(sets_info& sigma, std::vector<std::vector<double>> xvals) {
    sigma.fs = 0; sigma.ss = 0; sigma.ts = 0; sigma.lhs = 0;
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(i >= 1 && i <= 2*n && sigma.in_S[i] && !sigma.in_S[j]) {
                    sigma.fs += xvals[i][j];
                }
                if(j >= 1 && j <= 2*n && sigma.in_S[j] && !sigma.in_S[i]) {
                    sigma.fs += xvals[i][j];
                }
                if(i >= 1 && i <= 2*n && sigma.in_fs[i] && j >= 1 && j <= 2*n && sigma.in_ss[j]) {
                    sigma.fs += xvals[i][j];
                }
                if(i >= 1 && i <= 2*n && sigma.in_ts[i] && j >= 1 && j <= 2*n && sigma.in_S[j]) {
                    sigma.fs += xvals[i][j];
                }
            }
        }
    }
    
    sigma.lhs = sigma.fs - 2 * sigma.ss - 2 * sigma.ts;
}