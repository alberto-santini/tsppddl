#include <solver/bc/callbacks/vi_separator_generalised_order.h>

#include <chrono>

std::vector<IloRange> vi_separator_generalised_order::separate_valid_cuts() const {
    using namespace std::chrono;
    
    auto n = g.g[graph_bundle].n;
    auto cuts = std::vector<IloRange>();
    auto start_time = high_resolution_clock::now();
    
    for(auto i = 1; i <= n; i++) {
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<duration<double>>(current_time - start_time);
    
        if(elapsed_time.count() > params.bc.generalised_order.tilim) { break; }
        
        auto j1 = -1, j2 = -1;
        auto best_val_1 = 0.0, best_val_2 = 0.0;
        
        for(auto j = 1; j <= n; j++) {
            if(j != i) {
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
        }
        
        if(j1 == -1 && j2 == -1) { continue; }
        
        auto k1 = -1, k2 = -1;
        auto starting_val_1 = best_val_1, starting_val_2 = best_val_2;
        
        for(auto k = 1; k <= n; k++) {
            if(k != i) {
                if(j1 != -1) {
                    auto val_1 = starting_val_1 + sol.x[j1][n+k] + sol.x[k][n+i] + sol.x[n+k][j1] + sol.x[n+i][k] + sol.x[i][n+k];
                    
                    if(k != j1 && val_1 > best_val_1) {
                        k1 = k;
                        best_val_1 = val_1;
                    }
                }
                
                if(j2 != -1) {
                    auto val_2 = starting_val_2 + sol.x[j2][n+k] + sol.x[k][n+i] + sol.x[n+k][j2] + sol.x[n+i][k];
            
                    if(k != j2 && val_2 > best_val_2) {
                        k2 = k;
                        best_val_2 = val_2;
                    }
                }
            }
        }
        
        if(j1 != -1 && k1 != -1 && best_val_1 > 2 + ch::eps(2)) {
            IloExpr lhs(env);
            IloNum rhs = 2;
            
            auto col_index = 0;
            for(auto ii = 0; ii <= 2 * n + 1; ii++) {
                for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g.cost[ii][jj] >= 0) {
                        if(ii == i && jj == n + j1) {
                            lhs += x[col_index];
                        }
                        if(ii == j1 && jj == n + k1) {
                            lhs += x[col_index];
                        }
                        if(ii == k1 && jj == n + i) {
                            lhs += x[col_index];
                        }
                        if(ii == n + j1 && jj == i) {
                            lhs += x[col_index];
                        }
                        if(ii == n + k1 && jj == j1) {
                            lhs += x[col_index];
                        }
                        if(ii == n + i && jj == k1) {
                            lhs += x[col_index];
                        }
                        if(ii == i && jj == j1) {
                            lhs += x[col_index];
                        }
                        if(ii == i && jj == n + k1) {
                            lhs += x[col_index];
                        }
                        col_index++;
                    }
                }
            }
            
            IloRange cut;
            cut = (lhs <= rhs);
            
            cuts.push_back(cut);
        }
        
        if(j2 != -1 && k2 != -1 && best_val_2 > 2 + ch::eps(2)) {
            IloExpr lhs(env);
            IloNum rhs = 2;
            
            auto col_index = 0;
            for(auto ii = 0; ii <= 2 * n + 1; ii++) {
                for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g.cost[ii][jj] >= 0) {
                        if(ii == i && jj == n + j2) {
                            lhs += x[col_index];
                        }
                        if(ii == j2 && jj == n + k2) {
                            lhs += x[col_index];
                        }
                        if(ii == k2 && jj == n + i) {
                            lhs += x[col_index];
                        }
                        if(ii == n + j2 && jj == i) {
                            lhs += x[col_index];
                        }
                        if(ii == n + k2 && jj == j2) {
                            lhs += x[col_index];
                        }
                        if(ii == n + i && jj == k2) {
                            lhs += x[col_index];
                        }
                        if(ii == n + i && jj == n + j2) {
                            lhs += x[col_index];
                        }
                        col_index++;
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