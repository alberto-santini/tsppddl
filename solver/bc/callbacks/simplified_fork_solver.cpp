#include <solver/bc/callbacks/simplified_fork_solver.h>

std::vector<IloRange> SimplifiedForkSolver::separate_valid_cuts() const {
    auto n = g.g[graph_bundle].n;
    auto cuts = std::vector<IloRange>();
    
    for(auto i = 1; i <= 2*n; i++) {
        for(auto j = 1; j <= 2*n; j++) {
            if(sol.x[i][j] > eps) {
                auto inf_sets = scan_for_infork(i,j);
                auto outf_sets = scan_for_outfork(i,j);
                
                auto lhs_val_inf = 0.0;
                auto lhs_val_outf = 0.0;
                
                for(auto h : inf_sets.S) {
                    lhs_val_inf += sol.x[h][i];
                }
                for(auto k : inf_sets.T) {
                    lhs_val_inf += sol.x[i][k];
                }
                for(auto h : outf_sets.S) {
                    lhs_val_outf += sol.x[h][j];
                }
                for(auto k : outf_sets.T) {
                    lhs_val_outf += sol.x[j][k];
                }
                
                auto inf_violated = (lhs_val_inf > 1 + eps);
                auto outf_violated = (lhs_val_outf > 1 + eps);
                                
                IloExpr lhs_inf(env);
                IloExpr lhs_outf(env);
                IloNum rhs = 1;
                
                auto col_index = 0;
                for(auto ii = 0; ii <= 2 * n + 1; ii++) {
                    for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                        if(g.cost[ii][jj] >= 0) {
                            if(inf_violated){
                                if(std::find(inf_sets.S.begin(), inf_sets.S.end(), ii) != inf_sets.S.end() && jj == i) {
                                    lhs_inf += x[col_index];
                                }
                                if(ii == i && std::find(inf_sets.T.begin(), inf_sets.S.end(), jj) != inf_sets.S.end()) {
                                    lhs_inf += x[col_index];
                                }
                            }
                            if(outf_violated) {
                                if(std::find(outf_sets.S.begin(), outf_sets.S.end(), ii) != outf_sets.S.end() && jj == j) {
                                    lhs_outf += x[col_index];
                                }
                                if(ii == j && std::find(outf_sets.T.begin(), outf_sets.S.end(), jj) != outf_sets.S.end()) {
                                    lhs_outf += x[col_index];
                                }
                            }
                            col_index++;
                        }
                    }
                }
                
                if(inf_violated) {
                    cuts.push_back(lhs_inf <= rhs);
                }
                if(outf_violated) {
                    cuts.push_back(lhs_outf <= rhs);
                }
            }
        }
    }
    
    return cuts;
}

CutDefiningSets SimplifiedForkSolver::scan_for_infork(int i, int j) const {
    auto infork_S = std::vector<int>();
    auto infork_T = std::vector<int>();
    auto n = g.g[graph_bundle].n;
    
    for(auto k = 1; k <= 2 * n; k++) {
        if(k != i && k != j) {
            if(std::find(g.infeasible_3_paths.begin(), g.infeasible_3_paths.end(), std::vector<int>{i,j,k}) != g.infeasible_3_paths.end()) {
                infork_T.push_back(k);
            }
        }
    }
    
    for(auto h = 1; h <= 2 * n; h++) {
        if(h != j) {
            bool candidate = true;
            for(auto k : infork_T) {
                if(k == h || std::find(g.infeasible_3_paths.begin(), g.infeasible_3_paths.end(), std::vector<int>{h,j,k}) != g.infeasible_3_paths.end()) {
                    candidate = false;
                    break;
                }
            }
            if(candidate) {
                infork_S.push_back(h);
            }
        }
    }
    
    return CutDefiningSets(std::move(infork_S), std::move(infork_T));
}

CutDefiningSets SimplifiedForkSolver::scan_for_outfork(int i, int j) const {
    auto outfork_S = std::vector<int>();
    auto outfork_T = std::vector<int>();
    auto n = g.g[graph_bundle].n;
    
    for(auto h = 1; h <= 2 * n; h++) {
        if(h != i && h != j) {
            if(std::find(g.infeasible_3_paths.begin(), g.infeasible_3_paths.end(), std::vector<int>{h,i,j}) != g.infeasible_3_paths.end()) {
                outfork_S.push_back(h);
            }
        }
    }
    
    for(auto k = 1; k <= 2 * n; k++) {
        if(k != i) {
            bool candidate = true;
            for(auto h : outfork_S) {
                if(h == k || std::find(g.infeasible_3_paths.begin(), g.infeasible_3_paths.end(), std::vector<int>{h,i,k}) == g.infeasible_3_paths.end()) {
                    candidate = false;
                    break;
                }
            }
            if(candidate) {
                outfork_T.push_back(k);
            }
        }
    }
    
    return CutDefiningSets(std::move(outfork_S), std::move(outfork_T));
}