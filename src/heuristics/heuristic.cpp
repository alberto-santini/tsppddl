#include <heuristics/heuristic.h>

#include <iostream>
#include <numeric>

heuristic::heuristic(const tsp_graph& g) : g{g} {
    n = g.g[graph_bundle].n;
    remaining_requests = std::vector<int>(n);
    std::iota(remaining_requests.begin(), remaining_requests.end(), 1);
    
    p.path_v.reserve (2 * n + 2); p.load_v.reserve(2 * n + 2);
    p.path_v.push_back(0); p.path_v.push_back(2*n+1);
    p.load_v.push_back(0); p.load_v.push_back(0);
}