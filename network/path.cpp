#include <network/path.h>

#include <iostream>
#include <stdexcept>

Path::Path(const Graph& g, const std::vector<std::vector<int>>& x) {
    auto current_node = 0;
    auto current_load = 0;
    auto visited_nodes = std::vector<int>();
    auto n = g.g[graph_bundle].n;
    
    total_cost = 0;
    total_load = 0;
    
    path.reserve(2 * n + 2);
    load.reserve(2 * n + 2);
    path.push_back(0);
    load.push_back(0);
    
    visited_nodes.reserve(2 * n + 2);
        
    while(current_node != 2 * n + 1) {
        auto cycle = std::find(visited_nodes.begin(), visited_nodes.end(), current_node);
        if(cycle != visited_nodes.end()) {
            std::cerr << ">> Cycle: the path comes back to " << *cycle << std::endl;
            throw std::runtime_error("K-opt heuristic produced a path with a cycle!");
        }
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(std::abs(x[current_node][j] - 1) < 0.0001) {
                path.push_back(j);
                current_load += g.demand[j];
                load.push_back(current_load);
                if(g.demand[j] > 0) { total_load += g.demand[j]; }
                total_cost += g.cost[current_node][j];
                visited_nodes.push_back(current_node);
                current_node = j;
                break;
            }
        }
    }
    
    if(path.size() != (size_t)(2 * n + 2)) {
        std::cerr << "Path length: " << path.size() << " - expected: " << (2*n+2) << std::endl;
        std::cerr << "Path: ";
        for(auto i = 0u; i < path.size(); i++) {
            std::cerr << path[i] << " ";
        }
        std::cerr << std::endl;
        std::cerr << "X: " << std::endl;
        for(auto i = 0; i <= 2*n+1; i++) {
            for(auto j = 0; j <= 2*n+1; j++) {
                if(x[i][j] > 0.0) {
                    std::cerr << "x[" << i << "][" << j << "] = " << x[i][j] << std::endl;
                }
            }
        }
        throw std::runtime_error("K-opt heuristic produced a short path!");
    }
}

void Path::verify_feasible(const Graph& g) const {
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
    auto current_load = 0;
    auto visited_nodes = 1;
    auto visited_sources = std::vector<int>();
    
    if(path.size() != (size_t)(2 * n + 2)) {
        std::cout << "Wrong path length: " << path.size() << " vs. " << 2 * n + 2 << std::endl;
        std::cout << "Path: ";
        for(auto i = 0u; i < path.size(); i++) { std::cout << path.at(i) << " "; }
        std::cout << std::endl;
        throw std::runtime_error("Path length is != 2 * n + 2");
    }
    
    for(auto i = 1; i <= 2 * n + 1; i++) {
        auto current_node = path.at(i);
        visited_nodes++;
        
        if(current_node == 2 * n + 1 && visited_nodes < 2 * n + 2) {
            std::cout << "Reached 2n+1 after just " << visited_nodes << " nodes" << std::endl;
            throw std::runtime_error("Reached 2n+1 after too few nodes!");
        }
        
        if(current_node > 0 && current_node <= n) {
            visited_sources.push_back(current_node);
        }
        
        if(current_node > n && current_node < 2 * n + 1) {
            auto source_visited = false;
            for(auto j : visited_sources) {
                if(j == current_node - n) {
                    source_visited = true;
                }
            }
            if(!source_visited) {
                std::cout << "Visited " << current_node << " before " << current_node - n << std::endl;
                throw std::runtime_error("Visited a destination before its source!");
            }
        }
        
        if(current_load > std::min(Q, g.draught.at(current_node))) {
            std::cout << "Load upon entering " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g.draught.at(current_node) << ")" << std::endl;
            throw std::runtime_error("Wrong load!");
        }
        
        current_load += g.demand.at(current_node);
        
        if(current_load > std::min(Q, g.draught.at(current_node))) {
            std::cout << "Load upon exiting " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g.draught.at(current_node) << ")" << std::endl;
            throw std::runtime_error("Wrong load!");
        }
    }
}