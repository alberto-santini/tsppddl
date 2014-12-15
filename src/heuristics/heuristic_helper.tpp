template<class IC>
std::pair<bool, path> heuristic_helper::insert(const tsp_graph& g, const IC& insertion_comparator, int i, int x, int y, const path& p, double bc, double bl) {
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
    auto new_cost = p.total_cost, new_load = (p.total_load + g.demand[i]);
    
    if(x == y) {
        new_cost += -std::max(g.cost[p.path_v[x-1]][p.path_v[x]], 0) + g.cost[p.path_v[x-1]][i] + g.cost[i][n+i] + g.cost[n+i][p.path_v[x]];
    } else {
        new_cost += -std::max(g.cost[p.path_v[x-1]][p.path_v[x]], 0) - std::max(g.cost[p.path_v[y-1]][p.path_v[y]], 0) + g.cost[p.path_v[x-1]][i] + g.cost[i][p.path_v[x]] + g.cost[p.path_v[y-1]][n+i] + g.cost[n+i][p.path_v[y]];
    }
    
    if(insertion_comparator(bc, bl, new_cost, new_load)) {
        return std::make_pair(false, path());
    }
    
    path np;
    np.path_v = std::vector<int>(p.path_v.size() + 2, 0);
    np.load_v = std::vector<int>(p.load_v.size() + 2, 0);
    np.total_cost = new_cost;
    np.total_load = new_load;
    
    for(auto j = 0; j <= x-1; j++) {
        np.path_v[j] = p.path_v[j];
        np.load_v[j] = p.load_v[j];
    }
    
    np.path_v[x] = i;
    np.load_v[x] = np.load_v[x-1] + g.demand[i];
    
    if(x == y) {
        if(np.load_v[x] > std::min({g.draught[i], g.draught[n+i], Q})) {
            return std::make_pair(false, path());
        }
        
        np.path_v[x+1] = n+i;
        np.load_v[x+1] = np.load_v[x] + g.demand[n+i];
        
        if(np.load_v[x] > std::min({g.draught[n+i], g.draught[p.path_v[x]], Q})) {
            return std::make_pair(false, path());
        }
        
        for(auto j = (size_t)(x + 2); j < p.path_v.size() + 2; j++) {
            np.path_v[j] = p.path_v[j-2];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
            
            auto next_port_draught = (j <= p.path_v.size() ? g.draught[p.path_v[j-1]] : std::numeric_limits<int>::max());
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_pair(false, path());
            }
        }
    } else {
        if(np.load_v[x] > std::min({g.draught[i], g.draught[p.path_v[x]], Q})) {
            return std::make_pair(false, path());
        }
        
        for(auto j = (size_t)(x + 1); j <= (size_t)(y); j++) {
            np.path_v[j] = p.path_v[j-1];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
        
            auto next_port_draught = (j < (size_t)y ? g.draught[p.path_v[j]] : g.draught[n+i]);
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_pair(false, path());
            }
        }
    
        np.path_v[y+1] = n+i;
        np.load_v[y+1] = np.load_v[y] + g.demand[n+i];
    
        if(np.load_v[y+1] > std::min({g.draught[n+i], g.draught[p.path_v[y]], Q})) {
            return std::make_pair(false, path());
        }
    
        for(auto j = (size_t)(y + 2); j < p.path_v.size() + 2; j++) {
            np.path_v[j] = p.path_v[j-2];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
                
            auto next_port_draught = (j <= p.path_v.size() ? g.draught[p.path_v[j-1]] : std::numeric_limits<int>::max());
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_pair(false, path());
            }
        }
    }
    
    return std::make_pair(true, np);
}