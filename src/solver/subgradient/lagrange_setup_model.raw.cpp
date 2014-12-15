// ROWS
std::stringstream name;

for(auto i = 0; i <= 2*n; i++) {
    name.str(""); name << "cst_outdegree_" << i;
    outdegree.add(IloRange(env, 1.0, 1.0, name.str().c_str()));
}
for(auto i = 1; i <= 2*n + 1; i++) {
    name.str(""); name << "cst_indegree_" << i;
    indegree.add(IloRange(env, 1.0, 1.0, name.str().c_str()));
}
for(auto i = 1; i <= 2*n; i++) {
    name.str(""); name << "cst_load_" << i;
    load.add(IloRange(env, g.demand[i], g.demand[i], name.str().c_str()));
}
for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            name.str(""); name << "cst_y_lower_" << i << "_" << j;
            y_lower.add(IloRange(env, -IloInfinity, 0.0, name.str().c_str()));
            name.str(""); name << "cst_y_upper_" << i << "_" << j;
            y_upper.add(IloRange(env, 0.0, IloInfinity, name.str().c_str()));
            if(!params.sg.relax_mtz) {
                name.str(""); name << "cst_mtz_" << i << "_" << j;
                mtz.add(IloRange(env, -IloInfinity, 2*n, name.str().c_str()));
            }
        }
    }
}
initial_load.add(IloRange(env, 0.0, 0.0, "cst_initial_load"));
initial_order.add(IloRange(env, 0.0, 0.0, "cst_initial_order"));
if(!params.sg.relax_prec) {
    for(auto i = 1; i <= n; i++) {
        name.str(""); name << "cst_prec_" << i;
        prec.add(IloRange(env, -IloInfinity, 1.0, name.str().c_str()));
    }
}

// COLUMNS

for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            auto obj_coeff = 0.0;
            
            obj_coeff += g.cost[i][j];
            
            if(params.sg.relax_mtz) {
                obj_coeff += (2*n + 1) * lambda[i][j];
            }
            
            IloNumColumn col = obj(obj_coeff);
                                    
            for(auto ii = 0; ii <= 2*n; ii++) {
                auto coeff = 0;
                if(i == ii) { coeff = 1; }
                col += outdegree[ii](coeff);
            }
                        
            for(auto jj = 1; jj <= 2*n + 1; jj++) {
                auto coeff = 0;
                if(j == jj) { coeff = 1; }
                col += indegree[jj-1](coeff);
            }
                        
            for(auto ii = 1; ii <= 2*n; ii++) {
                col += load[ii-1](0);
            }
                        
            auto col_n = 0;
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
                            beta = std::min(std::min(Q - std::max(0, g.demand[j]), g.draught[i]), g.draught[j] - std::max(0, g.demand[j]));
                        }
                        
                        col += y_lower[col_n](alpha);
                        col += y_upper[col_n](beta);
                        
                        if(!params.sg.relax_mtz) {
                            if(i == ii && j == jj) {
                                col += mtz[col_n](2*n + 1);
                            } else {
                                col += mtz[col_n](0);
                            }
                        }
                        
                        col_n++;
                    }
                }
            }

            col += initial_load[0](0);
            col += initial_order[0](0);
            
            if(!params.sg.relax_prec) {
                for(auto ii = 1; ii <= n; ii++) {
                    col += prec[ii-1](0);
                }
            }
             
            name.str(""); name << "x_" << i << "_" << j;          
            IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool, name.str().c_str());
            variables_x.add(v);
            col.end();
        }
    }
}

for(auto i = 0; i <= 2*n + 1; i++) {
    for(auto j = 0; j <= 2*n + 1; j++) {
        if(g.cost[i][j] >= 0) {
            IloNumColumn col = obj(0);
                        
            for(auto ii = 0; ii <= 2*n; ii++) {
                col += outdegree[ii](0);
            }
            
            for(auto jj = 1; jj <= 2*n + 1; jj++) {
                col += indegree[jj-1](0);
            }
            
            for(auto ii = 1; ii <= 2*n; ii++) {
                auto coeff = 0;
                
                if(i == ii) { coeff = 1; }
                if(j == ii) { coeff = -1; }
                
                col += load[ii-1](coeff);
            }
            
            auto col_n = 0;
            for(auto ii = 0; ii <= 2*n + 1; ii++) {
                for(auto jj = 0; jj <= 2*n + 1; jj++) {
                    if(g.cost[ii][jj] >= 0) {
                        auto coeff = 0;
                        
                        if(i == ii && j == jj) {
                            coeff = -1;
                        }
                        
                        col += y_lower[col_n](coeff);
                        col += y_upper[col_n](coeff);
                        
                        if(!params.sg.relax_mtz) {
                            col += mtz[col_n](0.0);
                        }
                        
                        col_n++;
                    }
                }
            }
            
            if(!params.sg.relax_prec) {
                for(int ii = 1; ii <= n; ii++) {
                    col += prec[ii-1](0);
                }
            }
            
            col += initial_load[0](i == 0 ? 1 : 0);
            col += initial_order[0](0);
            
            name.str(""); name << "y_" << i << "_" << j;
            IloNumVar v(col, 0.0, Q, IloNumVar::Int, name.str().c_str());
            variables_y.add(v);
            col.end();
        }
    }
}

for(auto i = 0; i <= 2*n + 1; i++) {
    auto obj_coeff = 0.0;
    
    if(params.sg.relax_mtz) {
        for(int j = 0; j <= 2*n + 1; j++) {
            if(g.cost[i][j] >= 0) { obj_coeff += lambda[i][j]; }
            if(g.cost[j][i] >= 0) { obj_coeff -= lambda[j][i]; }
        }
    }
    
    if(params.sg.relax_prec) {
        if(i >= 1 && i <= n) {
            obj_coeff += mu[i];
        } else if(i >= n+1 && i <= 2*n) {
            obj_coeff -= mu[i-n];
        }
    }
    
    IloNumColumn col = obj(obj_coeff);
    
    for(auto ii = 0; ii <= 2*n; ii++) {
        col += outdegree[ii](0);
    }
    
    for(auto jj = 1; jj <= 2*n + 1; jj++) {
        col += indegree[jj-1](0);
    }
    
    for(auto ii = 1; ii <= 2*n; ii++) {
        col += load[ii-1](0);
    }
    
    auto col_n = 0;
    for(auto ii = 0; ii <= 2*n + 1; ii++) {
        for(auto jj = 0; jj <= 2*n + 1; jj++) {
            if(g.cost[ii][jj] >= 0) {
                col += y_lower[col_n](0);
                col += y_upper[col_n](0);
                
                if(!params.sg.relax_mtz) {
                    if(i == ii) {
                        col += mtz[col_n](1);
                    } else if(i == jj) {
                        col += mtz[col_n](-1);
                    } else {
                        col += mtz[col_n](0);
                    }
                }
                
                col_n++;
            }
        }
    }
    
    col += initial_load[0](0);
    col += initial_order[0](i == 0 ? 1 : 0);
    
    if(!params.sg.relax_prec) {
        for(auto ii = 1; ii <= n; ii++) {
            if(i == ii) {
                col += prec[ii-1](1);
            } else if(n + ii == i) {
                col += prec[ii-1](-1);
            } else {
                col += prec[ii-1](0);
            }
        }
    }
    
    name.str(""); name << "t_" << i;
    IloNumVar v(col, 0.0, 2 * n + 1, IloNumVar::Int, name.str().c_str());
    variables_tt.add(v);
    col.end();
}