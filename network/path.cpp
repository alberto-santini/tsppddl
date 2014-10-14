#include <network/path.h>

#include <iostream>
#include <stdexcept>

void Path::verify_feasible(std::shared_ptr<const Graph> g) {
    // std::cout << "***** Verifying the feasibility of a heuristic solution!" << std::endl;
    
    int n {g->g[graph_bundle].n};
    int Q {g->g[graph_bundle].capacity};
    int current_load {0};
    int visited_nodes {1};
    std::vector<int> visited_sources;
    
    if(path.size() != (size_t)(2 * n + 2)) {
        std::cout << "Wrong path length: " << path.size() << " vs. " << 2 * n + 2 << std::endl;
        std::cout << "Path: ";
        for(auto i = 0u; i < path.size(); i++) { std::cout << path[i] << " "; }
        std::cout << std::endl;
        throw std::runtime_error("Path length is != 2 * n + 2");
    }
    
    for(int i = 1; i <= 2 * n + 1; i++) {
        int current_node = path[i];
        visited_nodes++;
        
        if(current_node == 2 * n + 1 && visited_nodes < 2 * n + 2) {
            std::cout << "Reached 2n+1 after just " << visited_nodes << " nodes" << std::endl;
            throw std::runtime_error("Reached 2n+1 after too few nodes!");
        }
        
        if(current_node > 0 && current_node <= n) {
            visited_sources.push_back(current_node);
        }
        
        if(current_node > n && current_node < 2 * n + 1) {
            bool source_visited {false};
            for(int j : visited_sources) {
                if(j == current_node - n) {
                    source_visited = true;
                }
            }
            if(!source_visited) {
                std::cout << "Visited " << current_node << " before " << current_node - n << std::endl;
                throw std::runtime_error("Visited a destination before its source!");
            }
        }
        
        if(current_load > std::min(Q, g->draught[current_node])) {
            std::cout << "Load upon entering " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g->draught[current_node] << ")" << std::endl;
            throw std::runtime_error("Wrong load!");
        }
        
        current_load += g->demand[current_node];
        
        if(current_load > std::min(Q, g->draught[current_node])) {
            std::cout << "Load upon exiting " << current_node << " is " << current_load << " vs. Q (" << Q << ") or draught (" << g->draught[current_node] << ")" << std::endl;
            throw std::runtime_error("Wrong load!");
        }
    }
    
    // std::cout << "***** Solution feasible!" << std::endl;
}