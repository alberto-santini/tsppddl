// ROWS

for(int i = 0; i <= 2*n; i++) {
    outdegree.add(IloRange(env, 1.0, 1.0));
}
for(int i = 1; i <= 2*n + 1; i++) {
    indegree.add(IloRange(env, 1.0, 1.0));
}
for(int i = 1; i <= 2*n; i++) {
    load.add(IloRange(env, d[i], d[i]));
}
for(int i = 0; i <= 2*n + 1; i++) {
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) {
            y_lower.add(IloRange(env, -IloInfinity, 0.0));
            y_upper.add(IloRange(env, 0.0, IloInfinity));
        }
    }
}
initial_load.add(IloRange(env, 0.0, 0.0));
initial_order.add(IloRange(env, 0.0, 0.0));

// COLUMNS

for(int i = 0; i <= 2*n + 1; i++) {
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) {
            double obj_coeff {0};
            
            obj_coeff += c[i][j];
            obj_coeff += (2*n + 1) * lambda[i][j];
            IloNumColumn col = obj(obj_coeff);
                                    
            for(int ii = 0; ii <= 2*n; ii++) {
                int coeff {0};
                if(i == ii) { coeff = 1; }
                col += outdegree[ii](coeff);
            }
                        
            for(int jj = 1; jj <= 2*n + 1; jj++) {
                int coeff {0};
                if(j == jj) { coeff = 1; }
                col += indegree[jj-1](coeff);
            }
                        
            for(int ii = 1; ii <= 2*n; ii++) {
                col += load[ii-1](0);
            }
                        
            int col_n {0};
            for(int ii = 0; ii <= 2*n + 1; ii++) {
                for(int jj = 0; jj <= 2*n + 1; jj++) {
                    if(c[ii][jj] >= 0) {
                        int alpha {0};
                        int beta {0};
                        
                        if(i == ii && j == jj) {
                            if(i >= 1 && i <= n && j >= 1 && j <= n) { alpha = d[i]; }
                            if(i >= n+1 && i <= 2*n && j >= n+1 && j <= 2*n) { alpha = -d[j]; }
                            if(i >= 1 && i <= n && j >= n+1 && j <= 2*n) { alpha = d[i] - d[j]; }
                        }
                        beta = std::min(std::min(Q - std::max(0, d[j]), l[i]), l[j] - std::max(0, d[j]));
                        
                        col += y_lower[col_n](alpha);
                        col += y_upper[col_n](beta);
                        col_n++;
                    }
                }
            }
                        
            col += initial_load[0](0);
            col += initial_order[0](0);
                        
            IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool);
            variables_x.add(v);            
        }
    }
}

for(int i = 0; i <= 2*n + 1; i++) {
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) {
            IloNumColumn col = obj(0);
                        
            for(int ii = 0; ii <= 2*n; ii++) {
                col += outdegree[ii](0);
            }
            
            for(int jj = 1; jj <= 2*n + 1; jj++) {
                col += indegree[jj-1](0);
            }
            
            for(int ii = 1; ii <= 2*n; ii++) {
                int coeff {0};
                
                if(i == ii) { coeff = 1; }
                if(j == ii) { coeff = -1; }
                
                col += load[ii-1](coeff);
            }
            
            int col_n {0};
            for(int ii = 0; ii <= 2*n + 1; ii++) {
                for(int jj = 0; jj <= 2*n + 1; jj++) {
                    if(c[ii][jj] >= 0) {
                        col += y_lower[col_n](-1);
                        col += y_upper[col_n](-1);
                        col_n++;
                    }
                }
            }
            
            col += initial_load[0](i == 0 ? 1 : 0);
            col += initial_order[0](0);
            
            IloNumVar v(col, 0.0, Q, IloNumVar::Int);
            variables_y.add(v);
        }
    }
}

for(int i = 0; i < 2*n + 1; i++) {
    double obj_coeff {0};
    
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) { obj_coeff -= lambda[i][j]; }
        if(c[j][i] >= 0) { obj_coeff += lambda[j][i]; }
    }
    
    if(i >= 1 && i <= n) {
        obj_coeff -= mu[i];
        obj_coeff += mu[n+i];
    }
    
    IloNumColumn col = obj(obj_coeff);
    
    for(int ii = 0; ii <= 2*n; ii++) {
        col += outdegree[ii](0);
    }
    
    for(int jj = 1; jj <= 2*n + 1; jj++) {
        col += indegree[jj-1](0);
    }
    
    for(int ii = 1; ii <= 2*n; ii++) {
        col += load[ii-1](0);
    }
    
    int col_n {0};
    for(int ii = 0; ii <= 2*n + 1; ii++) {
        for(int jj = 0; jj <= 2*n + 1; jj++) {
            if(c[ii][jj] >= 0) {
                col += y_lower[col_n](0);
                col += y_upper[col_n](0);
                col_n++;
            }
        }
    }
    
    col += initial_load[0](0);
    col += initial_order[0](i == 0 ? 1 : 0);
    
    IloNumVar v(col, 0.0, 2 * n + 1, IloNumVar::Int);
    variables_tt.add(v);
}