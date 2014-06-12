template<class IC>
std::pair<bool, Path> HeuristicHelper::insert(const std::shared_ptr<const Graph> g, const IC insertion_comparator, const int i, const int x, const int y, const Path p, const double bc, const double bl) {
    int n {g->g[graph_bundle].n};
    int Q {g->g[graph_bundle].capacity};
    int new_cost {p.total_cost}, new_load {p.total_load + g->demand[i]};
    
    if(x == y) {
        new_cost += -std::max(g->cost[p.path[x-1]][p.path[x]], 0) + g->cost[p.path[x-1]][i] + g->cost[i][n+i] + g->cost[n+i][p.path[x]];
    } else {
        new_cost += -std::max(g->cost[p.path[x-1]][p.path[x]], 0) - std::max(g->cost[p.path[y-1]][p.path[y]], 0) + g->cost[p.path[x-1]][i] + g->cost[i][p.path[x]] + g->cost[p.path[y-1]][n+i] + g->cost[n+i][p.path[y]];
    }
    
    if(insertion_comparator(bc, bl, new_cost, new_load)) {
        return std::make_pair(false, Path());
    }
    
    Path np;
    np.path = std::vector<int>(static_cast<unsigned int>(p.path.size()) + 2, 0);
    np.load = std::vector<int>(static_cast<unsigned int>(p.load.size()) + 2, 0);
    np.total_cost = new_cost;
    np.total_load = new_load;
    
    for(int j = 0; j <= x-1; j++) {
        np.path[j] = p.path[j];
        np.load[j] = p.load[j];
    }
    
    np.path[x] = i;
    np.load[x] = np.load[x-1] + g->demand[i];
    
    if(x == y) {
        if(np.load[x] > std::min({g->draught[i], g->draught[n+i], Q})) {
            return std::make_pair(false, Path());
        }
        
        np.path[x+1] = n+i;
        np.load[x+1] = np.load[x] + g->demand[n+i];
        
        if(np.load[x] > std::min({g->draught[n+i], g->draught[p.path[x]], Q})) {
            return std::make_pair(false, Path());
        }
        
        for(int j = x + 2; j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + g->demand[np.path[j]];
            
            int next_port_draught {(j <= p.path.size() ? g->draught[p.path[j-1]] : std::numeric_limits<int>::max())};
            if(np.load[j] > std::min({g->draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    } else {
        if(np.load[x] > std::min({g->draught[i], g->draught[p.path[x]], Q})) {
            return std::make_pair(false, Path());
        }
        
        for(int j = x + 1; j <= y; j++) {
            np.path[j] = p.path[j-1];
            np.load[j] = np.load[j-1] + g->demand[np.path[j]];
        
            int next_port_draught {(j < y ? g->draught[p.path[j]] : g->draught[n+i])};
            if(np.load[j] > std::min({g->draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    
        np.path[y+1] = n+i;
        np.load[y+1] = np.load[y] + g->demand[n+i];
    
        if(np.load[y+1] > std::min({g->draught[n+i], g->draught[p.path[y]], Q})) {
            return std::make_pair(false, Path());
        }
    
        for(int j = y + 2; j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + g->demand[np.path[j]];
                
            int next_port_draught {(j <= p.path.size() ? g->draught[p.path[j-1]] : std::numeric_limits<int>::max())};
            if(np.load[j] > std::min({g->draught[np.path[j]], next_port_draught, Q})) {
                return std::make_pair(false, Path());
            }
        }
    }
    
    return std::make_pair(true, np);
}