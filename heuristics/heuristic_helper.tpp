template<typename InsertionPricer, typename InsertionComparator>
std::pair<bool, GenericPath> HeuristicHelper::feasible_and_improves(const int i, const int x, const int y, const int best_cost, const int best_load, const GenericPath& p, const std::shared_ptr<const RawData> rd, InsertionPricer insertion_pricer, InsertionComparator insertion_comparator) {
    int n = rd->n;
    int new_cost;
    int new_load;
    
    if(x == y) {
        new_cost = p.cost - rd->d[p.path[x-1]][p.path[x]] + rd->d[p.path[x-1]][i] + rd->d[i][n+i] + rd->d[n+i][p.path[x]];
    } else {
        new_cost = p.cost - rd->d[p.path[x-1]][p.path[x]] - rd->d[p.path[y-1]][p.path[y]] + rd->d[p.path[x-1]][i] + rd->d[i][p.path[x]] + rd->d[p.path[y-1]][n+i] + rd->d[n+i][p.path[y]];
    }
    
    new_load = p.total_load + rd->demand[i];
    
    if(insertion_comparator(best_cost, best_load, new_cost, new_load)) {
        return std::make_pair(false, GenericPath());
    }
    
    GenericPath np = GenericPath(std::vector<int>(p.path.size() + 2), std::vector<int>(p.load.size() + 2), 0, 0);
    np.cost = new_cost;
    np.total_load = new_load;
    
    for(int j = 0; j <= x-1; j++) {
        np.path[j] = p.path[j];
        np.load[j] = p.load[j];
    }
    
    np.path[x] = i;
    np.load[x] = np.load[x-1] + rd->demand[i];
    
    if(x != y) {
        if(np.load[x] > std::min({rd->draught[i], rd->draught[p.path[x]], rd->Q})) {
            return std::make_pair(false, GenericPath());
        }
        
        for(int j = x + 1; j <= y; j++) {
            np.path[j] = p.path[j-1];
            np.load[j] = np.load[j-1] + rd->demand[np.path[j]];
        
            int next_port_draught = (j < y ? rd->draught[p.path[j]] : rd->draught[n+i]);
            if(np.load[j] > std::min({rd->draught[np.path[j]], next_port_draught, rd->Q})) {
                return std::make_pair(false, GenericPath());
            }
        }
    
        np.path[y+1] = n+i;
        np.load[y+1] = np.load[y] + rd->demand[n+i];
    
        if(np.load[y+1] > std::min({rd->draught[n+i], rd->draught[p.path[y]], rd->Q})) {
            return std::make_pair(false, GenericPath());
        }
    
        for(int j = y + 2; j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + rd->demand[np.path[j]];
                
            int next_port_draught = (j <= p.path.size() ? rd->draught[p.path[j-1]] : std::numeric_limits<int>::max());
            if(np.load[j] > std::min({rd->draught[np.path[j]], next_port_draught, rd->Q})) {
                return std::make_pair(false, GenericPath());
            }
        }
    } else {
        if(np.load[x] > std::min({rd->draught[i], rd->draught[n+i], rd->Q})) {
            return std::make_pair(false, GenericPath());
        }
        
        np.path[x+1] = n+i;
        np.load[x+1] = np.load[x] + rd->demand[n+i];
        
        if(np.load[x] > std::min({rd->draught[n+i], rd->draught[p.path[x]], rd->Q})) {
            return std::make_pair(false, GenericPath());
        }
        
        for(int j = x + 2; j < p.path.size() + 2; j++) {
            np.path[j] = p.path[j-2];
            np.load[j] = np.load[j-1] + rd->demand[np.path[j]];
            
            int next_port_draught = (j <= p.path.size() ? rd->draught[p.path[j-1]] : std::numeric_limits<int>::max());
            if(np.load[j] > std::min({rd->draught[np.path[j]], next_port_draught, rd->Q})) {
                return std::make_pair(false, GenericPath());
            }
        }
    }
    
    return std::make_pair(true, np);
}