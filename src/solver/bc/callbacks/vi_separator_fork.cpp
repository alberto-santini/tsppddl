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
                        boost::optional<IloRange> cut = generate_cut(path, S, T);
            
                        if(cut) {
                            cuts.push_back(*cut);
                        }
                    }
                }
            }
        }
    }
    
    return cuts;
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