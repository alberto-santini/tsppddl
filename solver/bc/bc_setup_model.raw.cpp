// ROWS

for(int i = 0; i <= 2*n; i++) {
    outdegree.add(IloRange(env, 1.0, 1.0));
    outdegree[i].setName(("outdegree_" + std::to_string(i)).c_str());
}
for(int i = 1; i <= 2*n + 1; i++) {
    indegree.add(IloRange(env, 1.0, 1.0));
    indegree[i-1].setName(("indegree_" + std::to_string(i)).c_str());
}
for(int i = 1; i <= 2*n; i++) {
    load.add(IloRange(env, d[i], d[i]));
    load[i-1].setName(("load_" + std::to_string(i)).c_str());
}

int col_n {0};
for(int i = 0; i <= 2*n + 1; i++) {
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) {
            y_lower.add(IloRange(env, -IloInfinity, 0.0));
            y_lower[col_n].setName(("y_lower_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            y_upper.add(IloRange(env, 0.0, IloInfinity));
            y_upper[col_n++].setName(("y_upper_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
        }
    }
}

initial_load.add(IloRange(env, 0.0, 0.0));
initial_load[0].setName("initial_load");

if(tce) {
    col_n = 0;
    for(int i = 0; i <= 2*n + 1; i++) {
        for(int j = i + 1; j <= 2*n + 1; j++) {
            if(c[i][j] >= 0 && c[j][i] >= 0) {
                two_cycles_elimination.add(IloRange(env, -IloInfinity, 1.0));
                two_cycles_elimination[col_n++].setName(("tce_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            }
        }
    }
}

if(k_opt) {
    k_opt_constraint.add(IloRange(env, k_opt_rhs, IloInfinity));
    k_opt_constraint[0].setName("k_opt_constraint");
}

// COLUMNS

for(int i = 0; i <= 2*n + 1; i++) {
    for(int j = 0; j <= 2*n + 1; j++) {
        if(c[i][j] >= 0) {
            IloNumColumn col = obj(c[i][j]);
                                    
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
                            beta = std::min(std::min(Q - std::max(0, d[j]), l[i]), l[j] - std::max(0, d[j]));
                        }
                        
                        col += y_lower[col_n](alpha);
                        col += y_upper[col_n](beta);
                        col_n++;
                    }
                }
            }
                        
            col += initial_load[0](0);
            
            if(tce) {
                col_n = 0;
                for(int ii = 0; ii <= 2*n + 1; ii++) {
                    for(int jj = ii + 1; jj <= 2*n + 1; jj++) {
                        if(c[ii][jj] >= 0 && c[jj][ii] >= 0) {
                            int coeff {0};

                            if((i == ii && j == jj) || (i == jj && j == ii)) {
                                coeff = 1;
                            }

                            col += two_cycles_elimination[col_n++](coeff);
                        }
                    }
                }
            }
            
            if(k_opt) {
                col += k_opt_constraint[0](k_opt_lhs[i][j]);
            }

            IloNumVar v(col, 0.0, 1.0, IloNumVar::Bool, ("x_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
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
                        int coeff {0};

                        if(i == ii && j == jj) {
                            coeff = -1;
                        }
                        
                        col += y_lower[col_n](coeff);
                        col += y_upper[col_n](coeff);
                        col_n++;
                    }
                }
            }
            
            col += initial_load[0](i == 0 ? 1 : 0);
            
            if(tce) {
                col_n = 0;
                for(int ii = 0; ii <= 2*n + 1; ii++) {
                    for(int jj = ii + 1; jj <= 2*n + 1; jj++) {
                        if(c[ii][jj] >= 0 && c[jj][ii] >= 0) {
                            col += two_cycles_elimination[col_n++](0);
                        }
                    }
                }
            }
                        
            if(k_opt) {
                col += k_opt_constraint[0](0);
            }
            
            IloNumVar v(col, 0.0, Q, IloNumVar::Int, ("y_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            variables_y.add(v);
        }
    }
}