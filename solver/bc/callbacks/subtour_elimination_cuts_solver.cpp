#include <global.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <ratio>

SubtourEliminationCutsSolver::SubtourEliminationCutsSolver(std::shared_ptr<const Graph> g, ch::solution sol, IloEnv env, IloNumVarArray x, double eps) : g{g}, sol{sol}, env{env}, x{x}, eps{eps}, n{g->g[graph_bundle].n} {
    pi = ch::sets_info(n,
            bvec(2*n+2, false),
            bvec(2*n+2, false),
            ivec(2*n+2, -1),
            bvec(2*n+2, false), 0,
            bvec(2*n+2, false), 0,
            bvec(2*n+2, true), 0, 0);
            
    sigma = ch::sets_info(n,
            bvec(2*n+2, false),
            bvec(2*n+2, false),
            ivec(2*n+2, -1),
            bvec(2*n+2, true), 0,
            bvec(2*n+2, false), 0,
            bvec(2*n+2, false), 0, 0);
    
    // Nodes 0 and 2n+1 are there only for padding, but don't have any meaning        
    pi.in_ts[0] = false; pi.in_tabu[0] = true; pi.in_ts[2*n+1] = false; pi.in_tabu[2*n+1] = true;
    sigma.in_fs[0] = false; sigma.in_tabu[0] = true; sigma.in_fs[2*n+1] = false; sigma.in_tabu[2*n+1] = true;
            
    tot_number_of_iterations = 25;
    tabu_duration = 10;
}


std::vector<IloRange> SubtourEliminationCutsSolver::separate_valid_cuts() {
    using namespace std::chrono;
    using time_p =  high_resolution_clock::time_point;
    
    std::vector<IloRange> cuts;
    
    time_p start_time {high_resolution_clock::now()};
    
    for(int iter = 1; iter <= tot_number_of_iterations; iter++) {
        ch::sets_info best_pi = pi, best_sigma = sigma;
        int bn_pi = -1, bn_sigma = -1; // Best node \in (1...2n)
        
        for(int i = 1; i <= 2*n; i++) {
            ch::sets_info new_pi = pi, new_sigma = sigma;
                        
            add_or_remove_from_pi_sets(new_pi, i);
            recalculate_pi_sums(new_pi);
            add_or_remove_from_sigma_sets(new_sigma, i);
            recalculate_sigma_sums(new_sigma);
            
            if(i == pi.first_non_tabu() || (new_pi.lhs < best_pi.lhs && !pi.in_tabu[i] && !new_pi.empty_S())) {// Forbidding empty S
                best_pi = new_pi;
                bn_pi = i;
            }
            if(i == sigma.first_non_tabu() || (new_sigma.lhs < best_sigma.lhs && !sigma.in_tabu[i] && !new_sigma.empty_S())) {// Forbidding empty S
                best_sigma = new_sigma;
                bn_sigma = i;
            }
        }
        
        if(bn_pi == -1) { throw std::runtime_error("Best pi node can't be -1"); }
        
        update_info(pi, best_pi, bn_pi, iter);
        add_pi_cut_if_violated(cuts, pi);
        // Bonus: we can recycle set pi to add the groetschel pi cut
        add_groetschel_pi_cut_if_violated(cuts, pi);
        
        if(bn_sigma == -1) { throw std::runtime_error("Best sigma node can't be -1"); }
        
        update_info(sigma, best_sigma, bn_sigma, iter);
        add_sigma_cut_if_violated(cuts, sigma);
        // Bonus: we can recycle set sigma to add the groetschel sigma cut
        add_groetschel_sigma_cut_if_violated(cuts, sigma);
    }
    
    time_p end_time {high_resolution_clock::now()};
    duration<double> time_span {duration_cast<duration<double>>(end_time - start_time)};
    global::g_total_time_spent_separating_cuts += time_span.count();
    
    return cuts;
}

void SubtourEliminationCutsSolver::update_info(ch::sets_info& set, ch::sets_info best, int bn, int iter) {
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

void SubtourEliminationCutsSolver::add_pi_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info pi) {
    if(pi.lhs >= 2 - eps) { return; } // Cut not violated
    if(pi.empty_S()) { return; } // Empty S
    if(boost::count(pi.in_S, true) <= 1) { return; } // Trivial S gives a trivial inequality!
    
    IloExpr lhs(env);
    IloNum rhs = 2.0;
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(pi.is_in_S(i) && !pi.is_in_S(j)) {
                    lhs += x[col_index];
                }
                if(!pi.is_in_S(i) && pi.is_in_S(j)) {
                    lhs += x[col_index];
                }
                if(pi.is_in_fs(i) && pi.is_in_ts(j)) {
                    lhs += -2 * x[col_index];
                }
                if(pi.is_in_S(i) && pi.is_in_ss(j)) {
                    lhs += -2 * x[col_index];
                }
                col_index++;
            }
        }
    }
    
    IloRange cut;
    cut = (lhs >= rhs);
    cuts.push_back(cut);
}

void SubtourEliminationCutsSolver::add_groetschel_pi_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info pi) {
    if(pi.empty_S()) { return; }
    if(boost::count(pi.in_S, true) <= 1) { return; }
    
    std::vector<int> my_S;
    for(int i = 1; i <= 2*n; i++) { if(pi.in_S[i]) { my_S.push_back(i); } }
    
    std::cerr << "Checking groetschel pi cut" << std::endl;
    std::cerr << "\tSet S: "; for(int i : my_S) { std::cerr << i << " "; } std::cerr << std::endl;
    
    std::vector<double> outflow_S(my_S.size());
    for(int i = 1; i <= 2*n; i++) {
        for(int j = 1; j <= 2*n; j++) {
            if(sol.x[i][j] > 0) { outflow_S[i] += sol.x[i][j]; }
        }
    }
    
    int max_outflow_id = std::distance(outflow_S.begin(), std::max_element(outflow_S.begin(), outflow_S.end()));
    if(max_outflow_id != 1) {
        std::swap(*my_S.begin(), *(my_S.begin() + max_outflow_id));
    }
    
    std::random_shuffle(my_S.begin() + 1, my_S.end());
    
    std::cerr << "\tOrdered set S: "; for(int i : my_S) { std::cerr << i << " "; } std::cerr << std::endl;

    double lhs = calculate_groetschel_lhs_pi(my_S, pi);
    
    std::cerr << "\tLHS: " << lhs << std::endl;
    
    if(lhs > my_S.size() - 1) { actually_add_groetschel_pi_cut(cuts, my_S, pi); }
}

void SubtourEliminationCutsSolver::add_groetschel_sigma_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info sigma) {
}

void SubtourEliminationCutsSolver::actually_add_groetschel_pi_cut(std::vector<IloRange>& cuts, std::vector<int> my_S, ch::sets_info pi) {
    IloExpr lhs(env);
    IloNum rhs = my_S.size() - 1;
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                auto pos_i = std::find(my_S.begin(), my_S.end(), i);
                auto pos_j = std::find(my_S.begin(), my_S.end(), j);
                
                if(pos_i != my_S.end() && i != my_S.back()) { // First sum
                    if(j == *(pos_i + 1)) {
                        std::cerr << "\t\tx[" << i << "][" << j << "] (first sum)" << std::endl;
                        lhs += x[col_index];
                    }
                }
                
                if(i == my_S.back() && j == my_S[0]) { // x[i_h][i_1]
                    std::cerr << "\t\tx[" << i << "][" << j << "] (x)" << std::endl;
                    lhs += x[col_index];
                }
                
                if(pos_i != my_S.end() && pos_i > my_S.begin() && i != my_S.back() && j == my_S[0]) { // Second sum
                    std::cerr << "\t\tx[" << i << "][" << j << "] (second sum)" << std::endl;
                    lhs += 2 * x[col_index];
                }
                
                if(pos_i != my_S.end() && pos_i > my_S.begin() + 1 && i != my_S.back() && pos_j != my_S.end() && pos_j > my_S.begin() && pos_j < pos_i) { // Third sum
                    std::cerr << "\t\tx[" << i << "][" << j << "] (third sum)" << std::endl;
                    lhs += x[col_index];
                }
                
                if(i == my_S[0] && pi.is_in_S(j)) { // Fourth sum
                    std::cerr << "\t\tx[" << i << "][" << j << "] (fourth sum)" << std::endl;
                    lhs += x[col_index];
                }
                
                col_index++;
            }
        }
    }
    
    IloRange cut;
    cut = (lhs <= rhs);
    
    std::cerr << "Adding groetschel pi cut: " << cut << std::endl;
    
    cuts.push_back(cut);
}

void SubtourEliminationCutsSolver::actually_add_groetschel_sigma_cut(std::vector<IloRange>& cuts, std::vector<int> my_S, ch::sets_info sigma) {
}

double SubtourEliminationCutsSolver::calculate_groetschel_lhs_pi(std::vector<int> my_S, ch::sets_info pi) {
    double lhs {0};
    
    for(int k = 0; k < my_S.size() - 1; k++) {
        lhs += sol.x[my_S[k]][my_S[k+1]];
        if(k >= 1) {
            lhs += 2 * sol.x[my_S[k]][my_S[0]];
        }
        if(k >= 2) {
            for(int l = 1; l < k; l++) {
                lhs += sol.x[my_S[k]][my_S[l]];
            }
        }
    }
    for(int i = 1; i <= 2*n; i++) {
        if(pi.in_ss[i]) {
            lhs += sol.x[my_S[0]][i];
        }
    }
    lhs += sol.x[my_S.back()][my_S[0]];
    
    return lhs;
}

double SubtourEliminationCutsSolver::calculate_groetschel_lhs_sigma(std::vector<int> my_S, ch::sets_info sigma) {
    return 0;
}

void SubtourEliminationCutsSolver::add_sigma_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info sigma) {
    if(sigma.lhs >= 2 - eps) { return; } // Cut not violated
    if(sigma.empty_S()) { return; } // Empty S
    if(boost::count(sigma.in_S, true) <= 1) { return; } // Trivial S gives a trivial inequality!
    
    IloExpr lhs(env);
    IloNum rhs = 2.0;
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(sigma.is_in_S(i) && !sigma.is_in_S(j)) {
                    lhs += x[col_index];
                }
                if(!sigma.is_in_S(i) && sigma.is_in_S(j)) {
                    lhs += x[col_index];
                }
                if(sigma.is_in_fs(i) && sigma.is_in_ss(j)) {
                    lhs += -2 * x[col_index];
                }
                if(sigma.is_in_ts(i) && sigma.is_in_S(j)) {
                    lhs += -2 * x[col_index];
                }
                col_index++;
            }
        }
    }
    
    IloRange cut;
    cut = (lhs >= rhs);
    cuts.push_back(cut);
}

void SubtourEliminationCutsSolver::add_or_remove_from_pi_sets(ch::sets_info& pi, int i) {
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

void SubtourEliminationCutsSolver::add_or_remove_from_sigma_sets(ch::sets_info& sigma, int i) {
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

void SubtourEliminationCutsSolver::recalculate_pi_sums(ch::sets_info& pi) {
    pi.fs = 0; pi.ss = 0; pi.ts = 0; pi.lhs = 0;
        
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(pi.is_in_S(i) && !pi.is_in_S(j)) {
                    pi.fs += sol.x[i][j];
                }
                if(!pi.is_in_S(i) && pi.is_in_S(j)) {
                    pi.fs += sol.x[i][j];
                }
                if(pi.is_in_fs(i) && pi.is_in_ts(j)) {
                    pi.ss += sol.x[i][j];
                }
                if(pi.is_in_S(i) && pi.is_in_ss(j)) {
                    pi.ts += sol.x[i][j];
                }
            }
        }
    }
    
    pi.lhs = pi.fs - 2 * pi.ss - 2 * pi.ts;
}

void SubtourEliminationCutsSolver::recalculate_sigma_sums(ch::sets_info& sigma) {
    sigma.fs = 0; sigma.ss = 0; sigma.ts = 0; sigma.lhs = 0;
        
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(sigma.is_in_S(i) && !sigma.is_in_S(j)) {
                    sigma.fs += sol.x[i][j];
                }
                if(!sigma.is_in_S(i) && sigma.is_in_S(j)) {
                    sigma.fs += sol.x[i][j];
                }
                if(sigma.is_in_fs(i) && sigma.is_in_ss(j)) {
                    sigma.ss += sol.x[i][j];
                }
                if(sigma.is_in_ts(i) && sigma.is_in_S(j)) {
                    sigma.ts += sol.x[i][j];
                }
            }
        }
    }
    
    sigma.lhs = sigma.fs - 2 * sigma.ss - 2 * sigma.ts;
}