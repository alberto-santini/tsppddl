#include <solver/bc/callbacks/vi_separator_fork.h>

#include <stdexcept>

std::vector<IloRange> vi_separator_fork::separate_valid_cuts() {
    auto n = g.g[graph_bundle].n;
    auto cuts = std::vector<IloRange>();
        
    for(auto cur_node = 1; cur_node <= 2*n; cur_node++) {
        auto paths_to_check = std::vector<std::vector<int>>();                
        paths_to_check.push_back({cur_node});
        
        while(!paths_to_check.empty()) {
            auto path = paths_to_check.back();
            paths_to_check.pop_back();
            
            //  1)  Try to extend <path> if:
            //      a)  The new path length does not exceed 6
            //      b)  There are nodes != 2n+1 with positive flow from the last node of <path>
            extend_path(path, paths_to_check);
            
            if(path.size() >= 3u) {
                //  2)  Process current <path> = {<node 0>,...,<node l>} and create a set T
                //      of <j> s.t. {<node 0>,...,<node l>, <j>} is infeasible                
                auto T = create_set_T_for(path);
            
                if(T.size() > 0u) {
                    //  3)  Process current <path> and T --- and create a set S (containing <node 0>)
                    //      of <i> s.t. {<i>,<node 1>,...<node l>,<j>} is infeasible
                    auto S = create_set_S_for(path, T);
                    
                    if(S.size() > 0u) {
                        //  4)  Add the cut
                        auto cut = generate_cut(path, S, T);
            
                        if(cut) {
                            cuts.push_back(*cut);
                        } else if(params.bc.fork.lifted) {
                            auto lifted_cuts = try_to_lift(path, S, T);
                        
                            if(lifted_cuts) {
                                for(auto& c : *lifted_cuts) {
                                    cuts.push_back(c);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return cuts;
}

boost::optional<std::vector<IloRange>> vi_separator_fork::try_to_lift(const std::vector<int>& path, const std::vector<int>& S, const std::vector<int>& T) {
    auto n = g.g[graph_bundle].n;
    auto out_violated = false;
    auto in_violated = true;
    auto out_cut = IloRange();
    auto in_cut = IloRange();
    
    // Path is <node 1>, ..., <node l>;
    // S
    //  <node s>, <node 1>, ..., <node l> is infeasible for all s in S
    // T
    //  <node s>, <node 1>, ..., <node l>, <node t> is infeasible for all t in T
    
    // First, try to lift out-fork inequality:
    
    // Create sets T_1, ..., T_l
    auto Ts = std::vector<std::vector<int>>(path.size(), std::vector<int>());
    // We already know T_l: it's T
    Ts[path.size() - 1] = T;
    
    for(auto h = 0u; h < path.size() - 1; ++h) {
        // Create set T_h
        
        for(auto j = 1; j <= 2*n; ++j) {
            // See if j should go in T_h
            auto infeasible_for_all = true;
            
            if(j != path[h+1]) {
                for(const auto& i : S) {
                    if(i != j) {
                        // See if (i, <node 1>, ..., <node h>, j) is unfeasible
                        auto subpath = std::vector<int>(h+1 + 2);
                                
                        subpath[0] = i;
                        std::copy(path.begin(), path.begin() + h + 1, subpath.begin() + 1);
                        subpath[h+2] = j;
                
                        if(!is_infeasible(subpath)) {
                            infeasible_for_all = false;
                            break;
                        }
                    }
                }
                
                if(infeasible_for_all) {
                    Ts[h].push_back(j);
                }
            }
        }
    }
    
    auto lhsout = calculate_lifted_lhs_out(path, S, Ts);
    auto rhsout = path.size();

    if(lhsout.value > rhsout + ch::eps(rhsout)) {
        out_violated = true;

        IloExpr cplex_lhs(env);
        IloNum cplex_rhs = rhsout;

        auto col_index = 0;
        for(auto ii = 0; ii <= 2 * n + 1; ii++) {
            for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                if(g.cost[ii][jj] >= 0) {
                    if(std::find(lhsout.arcs.begin(), lhsout.arcs.end(), std::make_pair(ii, jj)) != lhsout.arcs.end()) {
                        cplex_lhs += x[col_index];
                    }
                    col_index++;
                }
            }
        }

        data.total_number_of_outfork_vi_added++;
        out_cut = (cplex_lhs <= cplex_rhs);
    }
    
    // Then, try to lift in-fork inequality:
    
    // Create sets S_1, ..., S_l
    auto Ss = std::vector<std::vector<int>>(path.size(), std::vector<int>());
    // We already know S_1: it's S
    Ss[0] = S;
    
    for(auto h = 1u; h < path.size(); ++h) {
        // Create set S_h
        for(auto i = 1; i <= 2*n; ++i) {
            // See if i should go in S_h
            auto infeasible_for_all = true;
            
            if(h > 0 && i != path[h-1]) {
                for(const auto& j : T) {
                    if(i != j) {
                        // See if (i, <node h>, ..., <node l>, j) is unfeasible
                        auto subpath = std::vector<int>(path.size() - h + 2);

                        subpath[0] = i;
                        std::copy(path.begin() + h, path.end(), subpath.begin() + 1);
                        subpath[path.size() - h + 1] = j;
                
                        if(!is_infeasible(subpath)) {
                            infeasible_for_all = false;
                            break;
                        }
                    }
                }
                
                if(infeasible_for_all) {
                    Ss[h].push_back(i);
                }
            }
        }
    }
    
    auto lhsin = calculate_lifted_lhs_in(path, Ss, T);
    auto rhsin = path.size();

    if(lhsin.value > rhsin + ch::eps(rhsin)) {
        in_violated = true;

        IloExpr cplex_lhs(env);
        IloNum cplex_rhs = rhsin;

        auto col_index = 0;
        for(auto ii = 0; ii <= 2 * n + 1; ii++) {
            for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                if(g.cost[ii][jj] >= 0) {
                    if(std::find(lhsin.arcs.begin(), lhsin.arcs.end(), std::make_pair(ii, jj)) != lhsin.arcs.end()) {
                        cplex_lhs += x[col_index];
                    }
                    col_index++;
                }
            }
        }

        data.total_number_of_infork_vi_added++;
        in_cut = (cplex_lhs <= cplex_rhs);
    }
    
    auto cuts = std::vector<IloRange>();
    
    if(out_violated) {
        cuts.push_back(out_cut);
    }
    
    if(in_violated) {
        cuts.push_back(in_cut);
    }
    
    if(cuts.size() > 0u) {
        return cuts;
    }
    
    return boost::none;
}

boost::optional<IloRange> vi_separator_fork::generate_cut(const std::vector<int>& path, const std::vector<int>& S, const std::vector<int>& T) const {
    auto n = g.g[graph_bundle].n;
    auto lhs = calculate_lhs(path, S, T);
    auto rhs = path.size();
    
    if(lhs.value > rhs + ch::eps(rhs)) {
        IloExpr cplex_lhs(env);
        IloNum cplex_rhs = rhs;
        
        auto col_index = 0;
        for(auto ii = 0; ii <= 2 * n + 1; ii++) {
            for(auto jj = 0; jj <= 2 * n + 1; jj++) {
                if(g.cost[ii][jj] >= 0) {
                    if(std::find(lhs.arcs.begin(), lhs.arcs.end(), std::make_pair(ii, jj)) != lhs.arcs.end()) {
                        cplex_lhs += x[col_index];
                    }
                    col_index++;
                }
            }
        }
        
        data.total_number_of_fork_vi_added++;
        
        IloRange cut;
        cut = (cplex_lhs <= cplex_rhs);
        
        return cut;
    }
    
    return boost::none;
}

vi_separator_fork::lhs_info vi_separator_fork::calculate_lhs(const std::vector<int>& path, const std::vector<int>& S, const std::vector<int>& T) const {
    auto value = 0.0;
    auto arcs = std::vector<std::pair<int,int>>();
    
    // All the arcs below are certain to exist, if the nodes made it into S, path, T
    
    for(auto i : S) {
        value += sol.x[i][path[0]];
        arcs.push_back(std::make_pair(i, path[0]));
    }
    
    for(auto k = 0u; k < path.size() - 1; k++) {
        value += sol.x[path[k]][path[k+1]];
        arcs.push_back(std::make_pair(path[k], path[k+1]));
    }
    
    for(auto j : T) {
        value += sol.x[path.back()][j];
        arcs.push_back(std::make_pair(path.back(), j));
    }
    
    return lhs_info(value, arcs);
}

vi_separator_fork::lhs_info vi_separator_fork::calculate_lifted_lhs_out(const std::vector<int>& path, const std::vector<int>& S, const std::vector<std::vector<int>>& Ts) const {
    auto value = 0.0;
    auto arcs = std::vector<std::pair<int, int>>();
    
    for(auto i : S) {
        value += sol.x[i][path[0]];
        arcs.push_back(std::make_pair(i, path[0]));
    }
    
    for(auto h = 0u; h < path.size() - 1; h++) {
        value += sol.x[path[h]][path[h+1]];
        arcs.push_back(std::make_pair(path[h], path[h+1]));
    }
    
    for(auto h = 0u; h < path.size(); h++) {
        for(auto j : Ts[h]) {
            value += sol.x[path[h]][j];
            arcs.push_back(std::make_pair(path[h], j));
        }
    }
    
    return lhs_info(value, arcs);
}

vi_separator_fork::lhs_info vi_separator_fork::calculate_lifted_lhs_in(const std::vector<int>& path, const std::vector<std::vector<int>>& Ss, const std::vector<int>& T) const {
    auto value = 0.0;
    auto arcs = std::vector<std::pair<int, int>>();
    
    for(auto h = 0u; h < path.size(); h++) {
        for(auto i : Ss[h]) {
            value += sol.x[i][path[h]];
            arcs.push_back(std::make_pair(i, path[h]));
        }
    }
    
    for(auto h = 0u; h < path.size() - 1; h++) {
        value += sol.x[path[h]][path[h+1]];
        arcs.push_back(std::make_pair(path[h], path[h+1]));
    }
    
    for(auto j : T) {
        value += sol.x[path[path.size() - 1]][j];
        arcs.push_back(std::make_pair(path[path.size() - 1], j));
    }
    
    return lhs_info(value, arcs);
}

void vi_separator_fork::extend_path(const std::vector<int>& path, std::vector<std::vector<int>>& paths_to_check) const {
    auto n = g.g[graph_bundle].n;
    
    if(path.size() < 6) {
        auto last_node_in_path = path.back();
        
        if(last_node_in_path != 2 * n + 1) {
            for(auto i = 1; i <= 2 * n; i++) {
                if(sol.x[last_node_in_path][i] > 0) {
                    if(std::find(path.begin(), path.end(), i) == path.end()) {
                        auto new_path = path;
                        new_path.push_back(i);
                        paths_to_check.push_back(new_path);
                    }
                }
            }
        }
    }
}

std::vector<int> vi_separator_fork::create_set_T_for(const std::vector<int>& path) {
    auto n = g.g[graph_bundle].n;
    auto T = std::vector<int>();
    
    for(auto i = 1; i <= 2 * n; i++) {
        if(std::find(path.begin(), path.end(), i) == path.end()) {
            auto new_path = path;
            new_path.push_back(i);
            
            if(is_infeasible(new_path)) {
                T.push_back(i);
            }
        }
    }
    
    return T;
}

std::vector<int> vi_separator_fork::create_set_S_for(std::vector<int>& path, const std::vector<int>& T) {
    auto n = g.g[graph_bundle].n;
    auto S = std::vector<int>();
    
    // Put first element of path in S
    S.push_back(path[0]);
    
    // Remove first element from path
    // path.erase(path.begin());
    std::vector<int>(path.begin() + 1, path.end()).swap(path); // This is more memory efficient, right?
        
    for(auto i = 1; i <= 2 * n; i++) {
        if(std::find(path.begin(), path.end(), i) == path.end() && std::find(S.begin(), S.end(), i) == S.end() && std::find(T.begin(), T.end(), i) == T.end()) {
            auto new_path_head = std::vector<int>();
            new_path_head.push_back(i);
            new_path_head.insert(new_path_head.end(), path.begin(), path.end());
                        
            auto i_goes_in_S = true;
            
            for(auto j : T) {
                auto new_path = new_path_head;
                new_path.push_back(j);
                
                if(!is_infeasible(new_path)) {
                    i_goes_in_S = false;
                    break;
                }
            }
            
            if(i_goes_in_S) {
                S.push_back(i);
            }
        }
    }
    
    return S;
}

bool vi_separator_fork::is_infeasible(const std::vector<int>& path) {
    if(path.size() == 2) {
        throw std::runtime_error("Path for which I should check the feasibility has size 2!");
    }
    
    auto in_cache = g.infeas_list.find(path);
    
    // Present in cache
    if(in_cache != g.infeas_list.end()) {
        return g.infeas_list.at(path);
    }
    
    // Not present in cache: must compute feasibility!
    
    auto n = g.g[graph_bundle].n;

    //  1)  For all delivery nodes in <path>, if the corresponding pickup node is not in <path>,
    //      then their delivery quantity must be added to the initial quantity; in the meanwhile
    //      we also check for violated precedence constraints
    
    auto Q = g.g[graph_bundle].capacity;
    auto initial_load = 0;
    
    for(auto i = 0u; i < path.size(); i++) {
        if(i >= (size_t)(n+1) && i <= (size_t)(2*n)) {
            // If the pickup node is after the delivery node => violated precedence constraint
            if(std::find(path.begin() + i + 1, path.end(), i-n) != path.end()) {
                g.infeas_list[path] = true;
                return true;
            }
            // If the pickup node is NOT before the delivery node, it means that we had that
            // load from the beginning of our path
            if(std::find(path.begin(), path.begin() + i, i-n) == path.begin() + i) {
                initial_load += g.demand.at(i-n);
            }
        }
    }
    
    //  2)  "Simulate" the ship sailing and check for violated capacity or draught constraints
    
    auto current_load = initial_load;
    
    for(auto i = 0u; i < path.size(); i++) {
        if(current_load > g.draught.at(path[i]) || current_load > Q) {
            g.infeas_list[path] = true;
            return true;
        } 
        
        current_load += g.demand[path[i]];
        
        if(current_load > g.draught.at(path[i]) || current_load > Q) {
            g.infeas_list[path] = true;
            return true;
        }
    }
    
    g.infeas_list[path] = false;
    return false;
}