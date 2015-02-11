#include <solver/bc/callbacks/vi_separator_capacity.h>

#include <algorithm>

vi_separator_capacity::vi_separator_capacity(const tsp_graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x) : g{g}, sol{sol}, env{env}, x{x} {
    auto n = g.g[graph_bundle].n;
    
    S = std::vector<int>();
    T = std::vector<int>();
    
    S.reserve(2*n);
    T.reserve(2*n);
}

std::vector<IloRange> vi_separator_capacity::separate_valid_cuts() {
    auto n = g.g[graph_bundle].n;
    auto cuts = std::vector<IloRange>();
    
    for(auto i = 1; i <= n; i++) {
        for(auto j = n+1; j <= 2*n; j++) {
            S = {i};
            T = {j};
            
            while(true) {
                auto bps = best_pickup_node_for_S();
                auto bds = best_delivery_node_for_S();
        
                if(!bps && !bds) {
                    break;
                }
                
                if(
                    !bps || (
                        bds &&
                        (*bps).flow < (*bds).flow &&
                        (*bds).flow >= 1
                    )
                ) {
                    // Add bds
                    S.push_back((*bds).node_n);
                    T.erase(std::remove(T.begin(), T.end(), (*bds).node_n), T.end());
                } else {
                    // Add bps
                    S.push_back((*bps).node_n);
                    T.erase(std::remove(T.begin(), T.end(), (*bps).node_n), T.end());
                }
        
                auto bpt = best_pickup_node_for_T();
                auto bdt = best_delivery_node_for_T();
                
                if(bdt || bpt) {
                    if(
                        !bdt || (
                            bpt &&
                            (*bdt).flow < (*bpt).flow &&
                            (*bpt).flow >= 1
                        )
                    ) {
                        // Add bpt
                        T.push_back((*bpt).node_n);
                    } else if(bdt) {
                        // Add bdt
                        T.push_back((*bdt).node_n);
                    }
                }
        
                auto lhs = calculate_lhs();
                auto rhs = calculate_rhs();
                        
                if(lhs >= rhs + ch::eps(rhs)) {
                    if(DEBUG) {
                        std::cerr << "vi_separator_capacity.cpp::separate_valid_cuts() \t Violated capacity cut: " << lhs << " >= " << rhs << " + " << ch::eps(rhs) << std::endl;
                    }
                    cuts.push_back(add_cut(rhs));
                }
            }
        }
    }
    
    return cuts;
}

IloRange vi_separator_capacity::add_cut(double rhs_val) const {
    auto n = g.g[graph_bundle].n;
    IloExpr lhs(env);
    IloNum rhs(rhs_val);
    
    auto col_index = 0;
    for(auto ii = 0; ii <= 2 * n + 1; ii++) {
        for(auto jj = 0; jj <= 2 * n + 1; jj++) {
            if(g.cost[ii][jj] >= 0) {
                if(std::find(S.begin(), S.end(), ii) != S.end()) {
                    if(std::find(S.begin(), S.end(), jj) != S.end()) {
                        lhs += x[col_index];
                    } else if(std::find(T.begin(), T.end(), jj) != T.end()) {
                        lhs += x[col_index];
                    }
                } else if(std::find(T.begin(), T.end(), ii) != T.end()) {
                    if(std::find(T.begin(), T.end(), jj) != T.end()) {
                        lhs += x[col_index];
                    }
                }
                col_index++;
            }
        }
    }
    
    IloRange cut;
    cut = (lhs <= rhs);
    
    return cut;
}

double vi_separator_capacity::calculate_lhs() const {
    auto lhs = 0.0;
        
    for(const auto& s1 : S) {
        for(const auto& s2 : S) {
            lhs += sol.x[s1][s2];
        }
        for(const auto& t : T) {
            lhs += sol.x[s1][t];
        }
    }
    
    for(const auto& t1 : T) {
        for(const auto& t2 : T) {
            lhs += sol.x[t1][t2];
        }
    }
        
    return lhs;
}

double vi_separator_capacity::calculate_rhs() const {
    auto n = g.g[graph_bundle].n;
    auto rhs = 0.0;
    auto demand_s = 0.0;
    auto demand_u = 0.0;
    auto denominator = 0.0;
    
    rhs += S.size() + T.size();
    
    for(const auto& s : S) {
        demand_s += g.demand.at(s);
    }
    
    for(const auto& t : T) {
        if(t >= n + 1 && t <= 2*n) {
            if(std::find(S.begin(), S.end(), t-n) == S.end() && std::find(T.begin(), T.end(), t-n) == T.end()) {
                demand_u += g.demand.at(t-n);
            }
        }
    }
    
    denominator = std::min(
        g.g[graph_bundle].capacity,
        g.draught[*std::max_element(S.begin(), S.end(),
            [this] (int n1, int n2) -> bool {
               return (g.draught[n1] < g.draught[n2]);
            }
        )]
    );
        
    rhs -= std::ceil((demand_s + demand_u) / denominator);
        
    return rhs;
}

boost::optional<vi_separator_capacity::best_node> vi_separator_capacity::best_pickup_node_for_T() const {
    auto n = g.g[graph_bundle].n;
    auto best_n = -1;
    auto best_f = 0.0;
    
    for(auto i = 1; i <= n; i++) {
        auto flow = 0.0;
        
        if(std::find(S.begin(), S.end(), i) != S.end()) {
            continue;
        }
        
        if(std::find(T.begin(), T.end(), i) != T.end()) {
            continue;
        }
        
        for(const auto& t : T) {
            flow += sol.x[t][i] + sol.x[i][t];
        }
        
        if(flow > best_f + ch::eps(best_f)) {
            best_f = flow;
            best_n = i;
        }
    }
    
    if(best_n != -1) {
        return best_node(best_n, best_f);
    } else {
        return boost::none;
    }
}

boost::optional<vi_separator_capacity::best_node> vi_separator_capacity::best_delivery_node_for_T() const {
    auto n = g.g[graph_bundle].n;
    auto best_n = -1;
    auto best_f = 0.0;
    
    for(auto i = n+1; i <= 2*n; i++) {
        auto flow = 0.0;
        
        if(std::find(S.begin(), S.end(), i) != S.end()) {
            continue;
        }
        
        if(std::find(T.begin(), T.end(), i) != T.end()) {
            continue;
        }
        
        for(const auto& t : T) {
            flow += sol.x[t][i] + sol.x[i][t];
        }
        
        if(flow > best_f + ch::eps(best_f)) {
            best_f = flow;
            best_n = i;
        }
    }
    
    if(best_n != -1) {
        return best_node(best_n, best_f);
    } else {
        return boost::none;
    }
}

boost::optional<vi_separator_capacity::best_node> vi_separator_capacity::best_pickup_node_for_S() const {
    auto n = g.g[graph_bundle].n;
    auto best_n = -1;
    auto best_f = 0.0;
        
    for(auto i = 1; i <= n; i++) {
        auto flow = 0.0;
                
        if(std::find(S.begin(), S.end(), i) != S.end()) {
            continue;
        }
        
        for(const auto& s : S) {
            flow += sol.x[s][i] + sol.x[i][s];
        }
                
        if(flow > best_f + ch::eps(best_f)) {
            best_f = flow;
            best_n = i;
        }
    }
    
    if(best_n != -1) {
        return best_node(best_n, best_f);
    } else {
        return boost::none;
    }
}

boost::optional<vi_separator_capacity::best_node> vi_separator_capacity::best_delivery_node_for_S() const {
    auto n = g.g[graph_bundle].n;
    auto best_n = -1;
    auto best_f = 0.0;
        
    for(auto i = n+1; i <= 2*n; i++) {
        auto flow = 0.0;
                
        if(std::find(S.begin(), S.end(), i) != S.end()) {
            continue;
        }
        
        for(const auto& s : S) {
            flow += sol.x[s][i] + sol.x[i][s];
        }
                
        if(flow > best_f) {
            best_f = flow;
            best_n = i;
        }
    }
    
    if(best_n != -1) {
        return best_node(best_n, best_f);
    } else {
        return boost::none;
    }
}