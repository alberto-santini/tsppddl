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
    
    auto xvals = sol.x;
    std::vector<IloRange> cuts;
    
    time_p start_time {high_resolution_clock::now()};
    
    for(int iter = 1; iter <= tot_number_of_iterations; iter++) {
        ch::sets_info best_pi = pi, best_sigma = sigma;
        int bn_pi = -1, bn_sigma = -1; // Best node \in (1...2n)
        
        std::vector<double> _tmp_lhs_pi(2*n+1, 0);
        std::vector<double> _tmp_lhs_sigma(2*n+1, 0);
        
        for(int i = 1; i <= 2*n; i++) {
            ch::sets_info new_pi = pi, new_sigma = sigma;
                        
            add_or_remove_from_pi_sets(new_pi, i);
            recalculate_pi_sums(new_pi, xvals);
            add_or_remove_from_sigma_sets(new_sigma, i);
            recalculate_sigma_sums(new_sigma, xvals);
            
            _tmp_lhs_pi[i] = new_pi.lhs;
            _tmp_lhs_sigma[i] = new_sigma.lhs;
            
            if(i == pi.first_non_tabu() || (new_pi.lhs < best_pi.lhs && !pi.in_tabu[i] && !new_pi.empty_S())) { // Explicitely forbidding empty S
                best_pi = new_pi;
                bn_pi = i;
            }
            if(i == sigma.first_non_tabu() || (new_sigma.lhs < best_sigma.lhs && !sigma.in_tabu[i] && !new_sigma.empty_S())) { // Explicitely forbidding empty S
                best_sigma = new_sigma;
                bn_sigma = i;
            }
        }
        
        std::cerr << "End of iteration " << iter << ". Outcome is: " << (pi.in_S[bn_pi] ? "remove " : "add ") << bn_pi << " for pi and " << (sigma.in_S[bn_sigma] ? "remove " : "add ") << bn_sigma << " for sigma." << std::endl;
        // std::cerr << "LHS for pi are: "; for(int i = 1; i <= 2*n; i++) { std::cerr << i << ", " << _tmp_lhs_pi[i] << "; "; } std::cerr << std::endl;
        // std::cerr << "LHS for sigma are: "; for(int i = 1; i <= 2*n; i++) { std::cerr << i << ", " << _tmp_lhs_sigma[i] << "; "; } std::cerr << std::endl;
        // std::cerr << "\tBest S for pi is: " << best_pi.print_S() << std::endl;
        // std::cerr << "\t\tFirst set: " << best_pi.print_fs() << std::endl;
        // std::cerr << "\t\tSecond set: " << best_pi.print_ss() << std::endl;
        // std::cerr << "\t\tThird set: " << best_pi.print_ts() << std::endl;
        // std::cerr << "\tBest S for sigma is: " << best_sigma.print_S() << std::endl;
        // std::cerr << "\t\tFirst set: " << best_sigma.print_fs() << std::endl;
        // std::cerr << "\t\tSecond set: " << best_sigma.print_ss() << std::endl;
        // std::cerr << "\t\tThird set: " << best_sigma.print_ts() << std::endl;
        
        if(bn_pi == -1) { throw std::runtime_error("Best pi node can't be -1"); } else {
            update_info(pi, best_pi, bn_pi, iter);
            add_pi_cut_if_violated(cuts, pi);
        }
        if(bn_sigma == -1) { throw std::runtime_error("Best sigma node can't be -1"); } else {
            update_info(sigma, best_sigma, bn_sigma, iter);
            add_sigma_cut_if_violated(cuts, sigma);
        }
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
    if(pi.lhs < 2 - eps) { return; } // Cut not violated
    if(pi.empty_S()) { return; } // Empty S
    if(boost::count(pi.in_S, true) <= 1) { return; } // Trivial S gives a trivial inequality!
    
    IloExpr lhs(env);
    IloNum rhs = 2.0;
    
    std::cerr << "\tAdding pi cut as lhs is " << pi.lhs << " < 2 and |S| > 1" << std::endl;
    
    std::string s_S = pi.print_S();
    std::string s_fs = pi.print_fs();
    std::string s_ss = pi.print_ss();
    std::string s_ts = pi.print_ts();
    
    std::ofstream cuts_file;
    cuts_file.open("valid_cuts_pi.txt", std::ios::out | std::ios::app);

    cuts_file << "S: " << s_S << std::endl;
    cuts_file << "fs: " << s_fs << std::endl;
    cuts_file << "ss: " << s_ss << std::endl;
    cuts_file << "ts: " << s_ts << std::endl;
    cuts_file << "sol: ";
    for(int i = 0; i <= 2 * n + 1; i++) { for(int j = 0; j <= 2 * n + 1; j++) { if(sol.x[i][j] > 0) { cuts_file << "x[" << i << "][" << j << "] = " << sol.x[i][j] << "; "; } } }
    cuts_file << std::endl;

    cuts_file << "cut: ";
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(pi.is_in_S(i) && !pi.is_in_S(j)) {
                    // cuts_file << "+ x[" << i << "][" << j << "] ";
                    lhs += x[col_index];
                }
                if(!pi.is_in_S(i) && pi.is_in_S(j)) {
                    // cuts_file << "+ x[" << i << "][" << j << "] ";
                    lhs += x[col_index];
                }
                if(pi.is_in_fs(i) && pi.is_in_ts(j)) {
                    // cuts_file << "-2 x[" << i << "][" << j << "] ";
                    lhs += -2 * x[col_index];
                }
                if(pi.is_in_S(i) && pi.is_in_ss(j)) {
                    // cuts_file << "-2 x[" << i << "][" << j << "] ";
                    lhs += -2 * x[col_index];
                }
                col_index++;
            }
        }
    }
    // cuts_file << std::endl << std::endl << std::endl;
    //
    // cuts_file.close();
    
    IloRange cut;
    cut = (lhs >= rhs);
    cuts.push_back(cut);
}

void SubtourEliminationCutsSolver::add_sigma_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info sigma) {
    if(sigma.lhs < 2 - eps) { return; } // Cut not violated
    if(sigma.empty_S()) { return; } // Empty S
    if(boost::count(sigma.in_S, true) <= 1) { return; } // Trivial S gives a trivial inequality!
    
    IloExpr lhs(env);
    IloNum rhs = 2.0;
    
    std::cerr << "\tAdding sigma cut as lhs is " << sigma.lhs << " < 2 and |S| > 1" << std::endl;
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(sigma.is_in_S(i) && !sigma.is_in_S(j)) {
                    // std::cerr << "\t\tx[" << i << "][" << j << "] in delta+ S" << std::endl;
                    lhs += x[col_index];
                }
                if(!pi.is_in_S(i) && pi.is_in_S(j)) {
                    // std::cerr << "\t\tx[" << i << "][" << j << "] in delta- S" << std::endl;
                    lhs += x[col_index];
                }
                if(sigma.is_in_fs(i) && sigma.is_in_ss(j)) {
                    // std::cerr << "\t\tx[" << i << "][" << j << "] in second sum" << std::endl;
                    lhs += -2 * x[col_index];
                }
                if(sigma.is_in_ts(i) && sigma.is_in_S(j)) {
                    // std::cerr << "\t\tx[" << i << "][" << j << "] in third sum" << std::endl;
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

void SubtourEliminationCutsSolver::recalculate_pi_sums(ch::sets_info& pi, std::vector<std::vector<double>> xvals) {
    pi.fs = 0; pi.ss = 0; pi.ts = 0; pi.lhs = 0;
    
    // std::cerr << "** Calculating LHS for pi (S: " << pi.print_S() << ")" << std::endl;
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(pi.is_in_S(i) && !pi.is_in_S(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta+ S, val: " << xvals[i][j] << std::endl; }
                    pi.fs += xvals[i][j];
                }
                if(!pi.is_in_S(i) && pi.is_in_S(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta- S, val: " << xvals[i][j] << std::endl; }
                    pi.fs += xvals[i][j];
                }
                if(pi.is_in_fs(i) && pi.is_in_ts(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta fs ts, val: " << xvals[i][j] << std::endl; }
                    pi.ss += xvals[i][j];
                }
                if(pi.is_in_S(i) && pi.is_in_ss(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta S ss, val: " << xvals[i][j] << std::endl; }
                    pi.ts += xvals[i][j];
                }
            }
        }
    }
    
    pi.lhs = pi.fs - 2 * pi.ss - 2 * pi.ts;
    // std::cerr << "**\tlhs: " << pi.fs << " - 2 * " << pi.ss << " - 2 * " << pi.ts << " = " << pi.lhs << std::endl;
}

void SubtourEliminationCutsSolver::recalculate_sigma_sums(ch::sets_info& sigma, std::vector<std::vector<double>> xvals) {
    sigma.fs = 0; sigma.ss = 0; sigma.ts = 0; sigma.lhs = 0;
    
    // std::cerr << "** Calculating LHS for sigma (S: " << sigma.print_S() << ")" << std::endl;
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(g->cost[i][j] >= 0) {
                if(sigma.is_in_S(i) && !sigma.is_in_S(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta+ S, val: " << xvals[i][j] << std::endl; }
                    sigma.fs += xvals[i][j];
                }
                if(!sigma.is_in_S(i) && sigma.is_in_S(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta- S, val: " << xvals[i][j] << std::endl; }
                    sigma.fs += xvals[i][j];
                }
                if(sigma.is_in_fs(i) && sigma.is_in_ss(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta fs ss, val: " << xvals[i][j] << std::endl; }
                    sigma.ss += xvals[i][j];
                }
                if(sigma.is_in_ts(i) && sigma.is_in_S(j)) {
                    // if(xvals[i][j] > 0) { std::cerr << "**\t\tx[" << i << "][" << j << "] In delta ts S, val: " << xvals[i][j] << std::endl; }
                    sigma.ts += xvals[i][j];
                }
            }
        }
    }
    
    sigma.lhs = sigma.fs - 2 * sigma.ss - 2 * sigma.ts;
    // std::cerr << "**\tlhs: " << sigma.fs << " - 2 * " << sigma.ss << " - 2 * " << sigma.ts << " = " << sigma.lhs << std::endl;
}