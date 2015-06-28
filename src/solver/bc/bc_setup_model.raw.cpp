auto min3 = [] (auto x, auto y, auto z) { return std::min(x, std::min(y,z)); };
auto max3 = [] (auto x, auto y, auto z) { return std::max(x, std::max(y,z)); };

// ROWS

for(auto i = 0; i <= 2*n; i++) {
    outdegree.add(IloRange(env, 1.0, 1.0));
    outdegree[i].setName(("outdegree_" + std::to_string(i)).c_str());
}
for(auto i = 1; i <= 2*n + 1; i++) {
    indegree.add(IloRange(env, 1.0, 1.0));
    indegree[i-1].setName(("indegree_" + std::to_string(i)).c_str());
}
for(auto i = 1; i <= 2*n; i++) {
    load.add(IloRange(env, g.demand[i], g.demand[i]));
    load[i-1].setName(("load_" + std::to_string(i)).c_str());
}

auto row_n = 0;
for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            y_lower.add(IloRange(env, -IloInfinity, 0.0));
            y_lower[row_n].setName(("y_lower_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            y_upper.add(IloRange(env, 0.0, IloInfinity));
            y_upper[row_n].setName(("y_upper_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            row_n++;
        }
    }
}

if(k_opt || params.bc.two_cycles_elim) {
    row_n = 0;
    for(auto i = 0; i <= 2*n + 1; i++) {
        for(auto j = i + 1; j <= 2*n + 1; j++) {
            if(g.cost[i][j] >= 0 && g.cost[j][i] >= 0) {
                two_cycles_elimination.add(IloRange(env, -IloInfinity, 1.0));
                two_cycles_elimination[row_n++].setName(("tce_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            }
        }
    }
}

if(k_opt || params.bc.subpath_elim) {
    row_n = 0;
    for(const auto& pi : g.infeas_list) {
        if((unsigned int)row_n < params.bc.max_infeas_subpaths) {
            if(pi.second) {
                std::stringstream name;
                name << "sub_";
                for(auto i : pi.first) { name << i << "_"; }
                name << "elim";
            
                subpath_elimination.add(IloRange(env, -IloInfinity, pi.first.size() - 2));
                subpath_elimination[row_n++].setName(name.str().c_str());
            }
        } else {
            break;
        }
    }
}

// COLUMNS
#include <boost/functional/hash.hpp>

auto ij_to_column = std::unordered_map<std::pair<int, int>, unsigned int, boost::hash<std::pair<int, int>>>();
auto col_n = 0u;

for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            IloNumColumn col = obj(g.cost[i][j]);

            for(auto ii = 0; ii <= 2*n; ii++) {
                if(i == ii) { col += outdegree[ii](1); }
            }
                        
            for(auto jj = 1; jj <= 2*n + 1; jj++) {
                if(j == jj) { col += indegree[jj-1](1); }
            }
                        
            auto row_n = 0;
            for(auto ii = 0; ii <= 2*n + 1; ii++) {
                for(auto jj = 0; jj <= 2*n + 1; jj++) {
                    if(g.cost[ii][jj] >= 0) {
                        auto alpha = 0;
                        auto beta = 0;
                        
                        if(i == ii && j == jj) {
                            if(i >= 1 && i <= n && j >= 1 && j <= n) { alpha = g.demand[i]; }
                            if(i >= n+1 && i <= 2*n && j >= n+1 && j <= 2*n) { alpha = -g.demand[j]; }
                            if(i >= 1 && i <= n && j >= n+1 && j <= 2*n) {
                                if(j != i+n) {
                                    alpha = g.demand[i] - g.demand[j];
                                } else {
                                    alpha = g.demand[i];
                                }
                            }

                            beta = min3(
                                g.draught[i] + std::min(0, g.demand[i]),
                                g.draught[j] - std::max(0, g.demand[j]),
                                Q - max3(0, -g.demand[i], g.demand[j])
                            );

                            // Old version (the new one is tighter than this):
                            // beta = std::min(std::min(Q - std::max(0, g.demand[j]), g.draught[i]), g.draught[j] - std::max(0, g.demand[j]));
                        }
                        
                        col += y_lower[row_n](alpha);
                        col += y_upper[row_n](beta);
                        row_n++;
                    }
                }
            }
            
            if(k_opt || params.bc.two_cycles_elim) {
                row_n = 0;
                for(auto ii = 0; ii <= 2*n + 1; ii++) {
                    for(auto jj = ii + 1; jj <= 2*n + 1; jj++) {
                        if(g.cost[ii][jj] >= 0 && g.cost[jj][ii] >= 0) {
                            if((i == ii && j == jj) || (i == jj && j == ii)) {
                                col += two_cycles_elimination[row_n](1);
                            }
                            row_n++;
                        }
                    }
                }
            }
            
            if(k_opt || params.bc.subpath_elim) {
                row_n = 0;
                for(const auto& pi : g.infeas_list) {
                    if((unsigned int)row_n < params.bc.max_infeas_subpaths) {
                        if(pi.second) {
                            for(auto path_pos = 0u; path_pos < pi.first.size() - 1; path_pos++) {
                                if(i == pi.first[path_pos] && j == pi.first[path_pos + 1]) {
                                    col += subpath_elimination[row_n](1);
                                }
                            }
                            row_n++;
                        }
                    } else {
                        break;
                    }
                }
            }
            
            if(k_opt) {
                col += k_opt_constraint(k_opt_lhs[i][j]);
            }
            
            IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool, ("x_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            variables_x.add(v);
            ij_to_column[std::make_pair(i,j)] = col_n++;
            col.end();
        }
    }
}

for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            IloNumColumn col = obj(0);
            
            for(auto ii = 1; ii <= 2*n; ii++) {
                if(i == ii) { col += load[ii-1](1); }
                if(j == ii) { col += load[ii-1](-1); }
            }
            
            auto row_n = 0;
            for(auto ii = 0; ii <= 2*n + 1; ii++) {
                for(auto jj = 0; jj <= 2*n + 1; jj++) {
                    if(g.cost[ii][jj] >= 0) {
                        if(i == ii && j == jj) {
                            col += y_lower[row_n](-1);
                            col += y_upper[row_n](-1);
                        }
                        row_n++;
                    }
                }
            }
            
            col += initial_load(i == 0 ? 1 : 0);
            
            IloNumVar v(col, 0.0, Q, IloNumVar::Int, ("y_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            variables_y.add(v);
            col.end();
        }
    }
}

// Model CLIQUE CUTS by row, because we don't know how many rows we will have

auto cn = [&ij_to_column] (int i, int j) -> unsigned int { return ij_to_column[std::make_pair(i,j)]; };

if(params.bc.clique_cuts) {
    row_n = 0;
    for(auto i = 1; i <= 2*n; i++) {
        for(auto j = 1; j <= 2*n; j++) {
            if(g.cost[i][j] >= 0) {
                // Cases 1)-2) in my notes
                if(i <= n && j <= n) {
                    IloExpr expr1(env);
                    expr1 = variables_x[cn(i,j)];
                
                    auto any_k1 = false;
                    for(auto k = 1; k <= n; k++) {
                        if(g.cost[j][k] >= 0 && g.demand[i] + g.demand[j] + g.demand[k] > std::min(Q, g.draught[k])) {
                            expr1 += variables_x[cn(j,k)];
                            any_k1 = true;
                        }
                    }
                    
                    if(any_k1) {
                        clique.add(expr1 <= 1.0);
                        clique[row_n++].setName(("clique_ppp_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                    
                    IloExpr expr2(env);
                    expr2 = variables_x[cn(i,j)];
                    
                    auto any_k2 = false;
                    for(auto k = n+1; k <= 2*n; k++) {
                        if(g.cost[j][k] >= 0 && k != n + i && k != n + j && (
                            g.demand[i] + g.demand[j] - g.demand[k] > min3(Q, g.draught[j], g.draught[k]) ||
                            g.demand[i] - g.demand[k] > std::min(g.draught[i], g.draught[j])
                        )) {
                            expr2 += variables_x[cn(j,k)];
                            any_k2 = true;
                        }
                    }
                    
                    if(any_k2) {
                        clique.add(expr2 <= 1.0);
                        clique[row_n++].setName(("clique_ppd_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                }
                
                // Cases 3)-4) in my notes
                if(i <= n && j > n && j != n+i) {
                    IloExpr expr1(env);
                    expr1 = variables_x[cn(i,j)];
                    
                    auto any_k1 = false;
                    for(auto k = 1; k <= 2*n; k++) {
                        if(g.cost[j][k] >= 0 && g.demand[i] + g.demand[k] > std::min(Q, g.draught[k])) {
                            expr1 += variables_x[cn(j,k)];
                            any_k1 = true;
                        }
                    }
                    
                    if(any_k1) {
                        clique.add(expr1 <= 1.0);
                        clique[row_n++].setName(("clique_pdp_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                    
                    IloExpr expr2(env);
                    expr2 = variables_x[cn(i,j)];
                    
                    auto any_k2 = false;
                    for(auto k = n+1; k <= 2*n; k++) {
                        if(g.cost[j][k] >= 0 && k != n + i && (
                            g.demand[i] - g.demand[j] - g.demand[k] > min3(Q, g.draught[i], g.draught[j]) ||
                            g.demand[i] - g.demand[k] > std::min(g.draught[k], g.draught[j]);
                        )) {
                            expr2 += variables_x[cn(j,k)];
                            any_k2 = true;
                        }
                    }
                    
                    if(any_k2) {
                        clique.add(expr2 <= 1.0);
                        clique[row_n++].setName(("clique_pdd_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                }
                
                // Cases 5)-6) in my notes
                if(i > n && j <= n) {
                    if(g.cost[j][i-n] >= 0) {
                        clique.add(variables_x[cn(i,j)] + variables_x[cn(j,i-n)] <= 1.0);
                        clique[row_n++].setName(("clique_dpp_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                    
                    IloExpr expr(env);
                    expr = variables_x[cn(i,j)];
                    
                    auto any_k = false;
                    for(auto k = n+1; k <= 2*n; k++) {
                        if(g.cost[j][k] >= 0 && k != n + j && - g.demand[i] - g.demand[k] > std::min(Q, g.draught[i])) {
                            expr += variables_x[cn(j,k)];
                            any_k = true;
                        }
                    }
                    
                    if(any_k) {
                        clique.add(expr <= 1.0);
                        clique[row_n++].setName(("clique_dpd_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                }
                
                // Cases 7)-8) in my notes
                if(i > n && j > n) {
                    if(g.cost[j][i-n] >= 0) {
                        clique.add(variables_x[cn(i,j)] + variables_x[cn(j,i-n)] <= 1.0);
                        clique[row_n++].setName(("clique_ddp_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                    
                    IloExpr expr(env);
                    expr = variables_x[cn(i,j)];
                    
                    auto any_k = false;
                    for(auto k = n+1; k <= 2*n; k++) {
                        if(g.cost[j][k] >= 0 && - g.demand[i] - g.demand[j] - g.demand[k] > std::min(Q, g.draught[i])) {
                            expr += variables_x[cn(j,k)];
                            any_k = true;
                        }
                    }
                    
                    if(any_k) {
                        clique.add(expr <= 1.0);
                        clique[row_n++].setName(("clique_ddd_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                    }
                }
            }
        }
    }
    
    clique_cuts_n = row_n;
}