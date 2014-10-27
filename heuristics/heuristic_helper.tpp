template<class IC>
std::pair<bool, Path> HeuristicHelper::insert(const Graph& g, const IC& insertion_comparator, int i, int x, int y, const Path& p, double bc, double bl) {
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
    auto new_cost = p.total_cost, new_load = (p.total_load + g.demand[i]);
    
    if(x == y) {
        new_cost += -std::max(g.cost[p.path[x-1]][p.path[x]], 0) + g.cost[p.path[x-1]][i] + g.cost[i][n+i] + g.cost[n+i][p.path[x]];
    } else {
        new_cost += -std::max(g.cost[p.path[x-1]][p.path[x]], 0) - std::max(g.cost[p.path[y-1]][p.path[y]], 0) + g.cost[p.path[x-1]][i] + g.cost[i][p.path[x]] + g.cost[p.path[y-1]][n+i] + g.cost[n+i][p.path[y]];
    }
    
    if(insertion_comparator(bc, bl, new_cost, new_load)) {
        return std::make_pair(false, Path());
    }
    
    Path np;
    np.path = std::vector<int>(p.path.size() + 2, 0);
    np.load = std::vector<int>(p.load.size() + 2, 0);
    np.total_cost = new_cost;
    np.total_load = new_load;
    
    for(auto j = 0; j <= x-1; j++) {
        np.path[j] = p.path[j];
        np.load[j] = p.load[j];
    }
    
    np.path[x] = i;
    np.load[x] = np.load[x-1] + g.demand[i];
    
    if(x == y) {
        if(np.load[x] > std::min({g.draught[i], g.draught[n+i], Q})) {
            return std::make_pair(false, Path());
        }
        
        np.path[x+1] = n+i;
        np.load[x+1] = np.load[x] + g.demand[n+i];
        
        if(np.load[x] > std::min({g.draught[n+i], g.draught[p.path[x]], Q})) {
            return std::make_pair(false, Path());
        }
        
        for(auto j = (size_t)(x + 2); j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + g.demand[np.path[j]];
            
            auto next_port_draught = (j <= p.path.size() ? g.draught[p.path[j-1]] : std::numeric_limits<int>::max());
            if(np.load[j] > std::min({g.draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    } else {
        if(np.load[x] > std::min({g.draught[i], g.draught[p.path[x]], Q})) {
            return std::make_pair(false, Path());
        }
        
        for(auto j = (size_t)(x + 1); j <= (size_t)(y); j++) {
            np.path[j] = p.path[j-1];
            np.load[j] = np.load[j-1] + g.demand[np.path[j]];
        
            auto next_port_draught = (j < (size_t)y ? g.draught[p.path[j]] : g.draught[n+i]);
            if(np.load[j] > std::min({g.draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    
        np.path[y+1] = n+i;
        np.load[y+1] = np.load[y] + g.demand[n+i];
    
        if(np.load[y+1] > std::min({g.draught[n+i], g.draught[p.path[y]], Q})) {
            return std::make_pair(false, Path());
        }
    
        for(auto j = (size_t)(y + 2); j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + g.demand[np.path[j]];
                
            auto next_port_draught = (j <= p.path.size() ? g.draught[p.path[j-1]] : std::numeric_limits<int>::max());
            if(np.load[j] > std::min({g.draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    }
    
    return std::make_pair(true, np);
}