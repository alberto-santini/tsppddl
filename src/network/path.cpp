#include <network/path.h>

#include <algorithm>
#include <stdexcept>

path::path(const tsp_graph& g, const std::vector<std::vector<int>>& x) {
    auto current_node = 0;
    auto current_load = 0;
    auto visited_nodes = std::vector<int>();
    auto n = g.g[graph_bundle].n;
    
    total_cost = 0;
    total_load = 0;
    
    path_v.reserve(2 * n + 2);
    load_v.reserve(2 * n + 2);
    path_v.push_back(0);
    load_v.push_back(0);
    
    visited_nodes.reserve(2 * n + 2);
        
    while(current_node != 2 * n + 1) {
        auto cycle = std::find(visited_nodes.begin(), visited_nodes.end(), current_node);
        if(cycle != visited_nodes.end()) {
            std::cerr << "path.cpp::path() \t Cycle: the path comes back to " << *cycle << std::endl;
        }
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(std::abs(x[current_node][j] - 1) < eps) {
                path_v.push_back(j);
                current_load += g.demand[j];
                load_v.push_back(current_load);
                if(g.demand[j] > 0) { total_load += g.demand[j]; }
                total_cost += g.cost[current_node][j];
                visited_nodes.push_back(current_node);
                current_node = j;
                break;
            }
        }
    }
    
    if(path_v.size() != (size_t)(2 * n + 2)) {
        std::cerr << "path.cpp::path() \t Path length: " << path_v.size() << " - expected: " << (2*n+2) << std::endl;
        std::cerr << "path.cpp::path() \t Path: ";
        for(auto i = 0u; i < path_v.size(); i++) {
            std::cerr << path_v[i] << " ";
        }
        std::cerr << std::endl;
    }
}

std::vector<std::vector<int>> path::get_x_values(int n) const {
    auto x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    
    for(auto i = 0u; i < path_v.size() - 1; i++) {
        x[path_v[i]][path_v[i+1]] = 1;
    }
    
    return x;
}

bool path::verify_feasible(const tsp_graph& g) const {
    auto n = g.g[graph_bundle].n;
    auto Q = g.g[graph_bundle].capacity;
    auto current_load = 0;
    auto visited_nodes = 1;
    auto visited_sources = std::vector<int>();
    
    if(path_v.size() != (size_t)(2 * n + 2)) {
        std::cerr << "path.cpp::verify_feasible() \t Wrong path length: " << path_v.size() << " vs. " << 2 * n + 2 << std::endl;
        std::cerr << "path.cpp::verify_feasible() \t Path: ";
        for(auto i = 0u; i < path_v.size(); i++) { std::cerr << path_v.at(i) << " "; }
        std::cerr << std::endl;
        return false;
    }
    
    for(auto i = 1; i <= 2 * n + 1; i++) {
        auto current_node = path_v.at(i);
        visited_nodes++;
        
        if(current_node == 2 * n + 1 && visited_nodes < 2 * n + 2) {
            std::cerr << "path.cpp::verify_feasible() \t Reached 2n+1 after just " << visited_nodes << " nodes vs. " << 2 * n + 2 << std::endl;
            return false;
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
                std::cerr << "path.cpp::verify_feasible() \t Visited " << current_node << " before its origin " << current_node - n << std::endl;
                return false;
            }
        }
        
        if(current_load > std::min(Q, g.draught.at(current_node))) {
            std::cerr << "path.cpp::verify_feasible() \t Load upon entering " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g.draught.at(current_node) << ")" << std::endl;
            return false;
        }
        
        current_load += g.demand.at(current_node);
        
        if(current_load > std::min(Q, g.draught.at(current_node))) {
            std::cerr << "path.cpp::verify_feasible() \t Load upon exiting " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g.draught.at(current_node) << ")" << std::endl;
            return false;
        }
    }
    
    return true;
}

void path::print(std::ostream& where) const {
    std::copy(path_v.begin(), path_v.end(), std::ostream_iterator<int>(where, " "));
}

bool path::operator==(const path& other) const {
    return path_v == other.path_v;
}