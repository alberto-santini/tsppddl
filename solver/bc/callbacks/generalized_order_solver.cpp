#include <solver/bc/callbacks/generalized_order_solver.h>

std::vector<IloRange> GeneralizedOrderSolver::separate_valid_cuts() {
    auto n = g->g[graph_bundle].n;
    std::vector<IloRange> cuts;
    
    for(auto i = 1; i <= 2*n; i++) {
        auto j1 = -1, j2 = -1;
        auto best_val_1 = 0.0, best_val_2 = 0.0;
        
        for(auto j = 1; j <= 2*n; j++) {
            auto val_1 = sol.x[i][n+j] + sol.x[n+j][i] + sol.x[i][j];
            auto val_2 = sol.x[i][n+j] + sol.x[n+j][i] + sol.x[n+i][n+j];
            
            if(val_1 > best_val_1) {
                j1 = j;
                best_val_1 = val_1;
            }
            
            if(val_2 > best_val_2) {
                j2 = j;
                best_val_2 = val_2;
            }
        }
        
        if(j1 == -1 && j2 == -1) { continue; }
        
        auto k1 = -1, k2 = -1;
        for(auto k = 1; k <= 2*n; k++) {
            auto val_1 = best_val_1 + sol.x[j1][n+k] + sol.x[k][n+i] + sol.x[n+k][j1] + sol.x[n+i][k] + sol.x[i][n+k];
            auto val_2 = best_val_2 + sol.x[j1][n+k] + sol.x[k][n+i] + sol.x[n+k][j2] + sol.x[n+i][k];
            
            if(val_1 > best_val_1) {
                k1 = k;
                best_val_1 = val_1;
            }
            
            if(val_2 > best_val_2) {
                k2 = k;
                best_val_2 = val_2;
            }
        }
        
        if(k1 != -1 && best_val_1 > 2 + eps) {
            IloExpr lhs(env);
            IloNum rhs = 2;
            
            int col_index {0};
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g->cost[ii][jj] >= 0) {
                        if(ii == i && jj == n + j1) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == j1 && jj == n + k1) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == k1 && jj == n + i) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + j1 && jj == i) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + k1 && jj == j1) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + i && jj == k1) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == i && jj == j1) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == i && jj == n + k1) {
                            lhs += x[col_index];
                            continue;
                        }
                    }
                }
            }
            
            IloRange cut;
            cut = (lhs <= rhs);
            
            cuts.push_back(cut);
        }
        
        if(k2 != -1 && best_val_2 > 2 + eps) {
            IloExpr lhs(env);
            IloNum rhs = 2;
            
            int col_index {0};
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g->cost[ii][jj] >= 0) {
                        if(ii == i && jj == n + j2) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == j2 && jj == n + k2) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == k2 && jj == n + i) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + j2 && jj == i) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + k2 && jj == j2) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + i && jj == k2) {
                            lhs += x[col_index];
                            continue;
                        }
                        if(ii == n + i && jj == n + j2) {
                            lhs += x[col_index];
                            continue;
                        }
                    }
                }
            }
            
            IloRange cut;
            cut = (lhs <= rhs);
            
            cuts.push_back(cut);
        }
    }
    
    return cuts;
}