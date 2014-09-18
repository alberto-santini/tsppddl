#include <heuristics/heuristic.h>

#include <iostream>
#include <numeric>

Heuristic::Heuristic(const std::shared_ptr<const Graph>& g) : g{g} {
    n = g->g[graph_bundle].n;
    remaining_requests = std::vector<int>(n);
    std::iota(remaining_requests.begin(), remaining_requests.end(), 1);
    
    p.path.reserve (2 * n + 2); p.load.reserve(2 * n + 2);
    p.path.push_back(0); p.path.push_back(2*n+1);
    p.load.push_back(0); p.load.push_back(0);
}

void Heuristic::print_requests() const {
    for(const int& r : remaining_requests) {
        std::cout << r << " ";
    }
    std::cout << std::endl;
}