#ifndef INSERTION_SCORER_H
#define INSERTION_SCORER_H

#include <network/tsp_graph.h>
#include <network/path.h>

#include <iostream>

template<class PS>
struct insertion_scorer {
    using result = std::tuple<bool, double, path>;
    
    const PS& p_scorer;
    
    insertion_scorer(const PS& p_scorer) : p_scorer(p_scorer) {}
    
    // Place request i with origin in position x and destination in position y
    result operator()(const tsp_graph& g, const path& p, int i, unsigned int x, unsigned int y) const;
};

template<class PS>
typename insertion_scorer<PS>::result insertion_scorer<PS>::operator()(const tsp_graph& g, const path& p, int i, unsigned int x, unsigned int y) const {
    assert(x <= y && y <= p.length());
    
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
    auto new_cost = p.total_cost;
    auto new_load = (p.total_load + g.demand[i]);
    double score = std::numeric_limits<double>::lowest();
    
    if(x == y) {
        new_cost += -std::max(g.cost[p.path_v[x-1]][p.path_v[x]], 0) + g.cost[p.path_v[x-1]][i] + g.cost[i][n+i] + g.cost[n+i][p.path_v[x]];
    } else {
        new_cost += -std::max(g.cost[p.path_v[x-1]][p.path_v[x]], 0) - std::max(g.cost[p.path_v[y-1]][p.path_v[y]], 0) + g.cost[p.path_v[x-1]][i] + g.cost[i][p.path_v[x]] + g.cost[p.path_v[y-1]][n+i] + g.cost[n+i][p.path_v[y]];
    }
    
    path np;
    np.path_v = std::vector<int>(p.path_v.size() + 2, 0);
    np.load_v = std::vector<int>(p.load_v.size() + 2, 0);
    np.total_cost = new_cost;
    np.total_load = new_load;
    
    for(auto j = 0u; j <= x-1; j++) {
        np.path_v[j] = p.path_v[j];
        np.load_v[j] = p.load_v[j];
    }

    np.path_v[x] = i;
    np.load_v[x] = np.load_v[x-1] + g.demand[i];

    if(x == y) {
        if(np.load_v[x] > std::min({g.draught[i], g.draught[n+i], Q})) {
            return std::make_tuple(false, score, np);
        }
    
        np.path_v[x+1] = n+i;
        np.load_v[x+1] = np.load_v[x] + g.demand[n+i];
    
        if(np.load_v[x] > std::min({g.draught[n+i], g.draught[p.path_v[x]], Q})) {
            return std::make_tuple(false, score, np);
        }
    
        for(auto j = x + 2; j < p.path_v.size() + 2; j++) {
            np.path_v[j] = p.path_v[j-2];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
        
            auto next_port_draught = (j <= p.path_v.size() ? g.draught[p.path_v[j-1]] : std::numeric_limits<int>::max());
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_tuple(false, score, np);
            }
        }
    } else {
        if(np.load_v[x] > std::min({g.draught[i], g.draught[p.path_v[x]], Q})) {
            return std::make_tuple(false, score, np);
        }
    
        for(auto j = x + 1; j <= y; j++) {
            np.path_v[j] = p.path_v[j-1];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
    
            auto next_port_draught = (j < y ? g.draught[p.path_v[j]] : g.draught[n+i]);
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_tuple(false, score, np);
            }
        }

        np.path_v[y+1] = n+i;
        np.load_v[y+1] = np.load_v[y] + g.demand[n+i];

        if(np.load_v[y+1] > std::min({g.draught[n+i], g.draught[p.path_v[y]], Q})) {
            return std::make_tuple(false, score, np);
        }

        for(auto j = y + 2; j < p.path_v.size() + 2; j++) {
            np.path_v[j] = p.path_v[j-2];
            np.load_v[j] = np.load_v[j-1] + g.demand[np.path_v[j]];
            
            auto next_port_draught = (j <= p.path_v.size() ? g.draught[p.path_v[j-1]] : std::numeric_limits<int>::max());
            if(np.load_v[j] > std::min({g.draught[np.path_v[j]], next_port_draught, Q})) {
                return std::make_tuple(false, score, np);
            }
        }
    }
    
    score = p_scorer(g, np);    
    return std::make_tuple(true, score, np);
}

#endif