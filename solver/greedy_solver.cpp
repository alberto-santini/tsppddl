#ifndef GREEDY_SOLVER_CPP
#define GREEDY_SOLVER_CPP

#include <algorithm>
#include <limits>
#include <ctime>

#include <solver/greedy_solver.h>

void GreedySolver::solve() const {
    Path p; double c; int n;
    std::tie(p, c, n) = get_best_path();
    
    if(c < 0) {
        std::cout << "The heuristic couldn't generate any path." << std::endl;
    } else {
        std::cout << "HEURISTIC objective value: " << c << std::endl;
        std::cout << "HEURISTIC generated " << n << " paths" << std::endl;
        std::cout << "The solution (port visit order) is: ";
        graph->print_path(p);
    }
}

std::tuple<Path, double, int> GreedySolver::get_best_path() const {
    Path best;
    double best_cost = std::numeric_limits<double>::max();
    int feasible_paths = 0;
    
    for(int i = 0; i < runs_number; i++) {
        Path p = generate_path();
        
        // std::cout << "Got a path of length " << p.size() << " and cost " << graph->cost(p) << std::endl;
        
        if(p.size() > 0) {
            feasible_paths++;
        }
        
        if(p.size() > 0 && graph->cost(p) < best_cost) {
            // std::cout << "\tIt's the new best path!" << std::endl;
            std::cout << "." << std::flush;
            best = p;
            best_cost = graph->cost(p);
        }
    }
    
    std::cout << std::endl;
    
    if(best.size() > 0) {
        return std::make_tuple(best, best_cost, feasible_paths);
    } else {
        return std::make_tuple(Path(), -1, 0);
    }
}

Path GreedySolver::generate_path() const {
    srand(std::time(0));
    
    Vertex current = graph->get_vertex(0);
    Vertex destination = graph->get_vertex(2 * graph->g[graph_bundle].num_requests + 1);
    
    std::vector<std::shared_ptr<Node>> visited_nodes;
    visited_nodes.push_back(graph->g[current]);
    
    int load = 0;
    bool success = false;
    Path p;
    
    std::vector<std::shared_ptr<Node>> visitable_origins = updated_visitable_origins(visited_nodes, load);
    
    while(current != destination) {
        std::vector<Edge> feasible_out_edges;
        std::vector<Edge> feasible_reserve_out_edges;
        oeit oei, oei_end;
        
        // std::cout << "\tCurrent node: " << graph->g[current]->id << std::endl;
        
        for(std::tie(oei, oei_end) = out_edges(current, graph->g); oei != oei_end; ++oei) {
            std::shared_ptr<Arc> arc = graph->g[*oei];
            std::shared_ptr<Node> dest = graph->g[target(*oei, graph->g)];
            
            // std::cout << "\t\tConsidering arc to " << dest->id << std::endl;
            
            if(contains(visited_nodes, dest->id)) {
                // Already visited
                // std::cout << "\t\t\tDiscarding: already visited" << std::endl;
                continue;
            }
            
            if(dest->id > graph->g[graph_bundle].num_requests && !contains(visited_nodes, dest->id - graph->g[graph_bundle].num_requests)) {
                // Destination but source not visited
                // std::cout << "\t\t\tDiscarding: source not visited" << std::endl;
                continue;
            }
            
            if(load > dest->port->draught || load + dest->demand > dest->port->draught || load > graph->g[graph_bundle].ship_capacity || load + dest->demand > graph->g[graph_bundle].ship_capacity) {
                // Draught or capacity constraints are violated
                // std::cout << "\t\t\tDiscarding: capacity violated" << std::endl;
                continue;
            }
            
            if(dest->id == 2 * graph->g[graph_bundle].num_requests + 1 && visited_nodes.size() != 2 * graph->g[graph_bundle].num_requests + 1) {
                // Service all requests before getting back
                // std::cout << "\t\t\tDiscarding: not all requests serviced yet" << std::endl;
                continue;
            }
            
            if(dest->demand > 0) { // For origin nodes, compute the number of other origins they make non visitable
                std::vector<std::shared_ptr<Node>> new_visited_nodes = visited_nodes;
                new_visited_nodes.push_back(dest);
                std::vector<std::shared_ptr<Node>> new_visitable_origins = updated_visitable_origins(new_visited_nodes, load + dest->demand);
                
                int n_visited_origins = 0;
                for(auto& n : visited_nodes) {
                    if(n->demand > 0) {
                        n_visited_origins++;
                    }
                }
                
                int n_unvisited_origins = graph->g[graph_bundle].num_requests - n_visited_origins;
                int n_unvisited_visitable_origins = new_visitable_origins.size();
                
                double ratio = (double)n_unvisited_visitable_origins / (double)n_unvisited_origins;
                
                if(ratio < visitable_coeff) {
                    // Ratio not satisfied
                    // std::cout << "\t\t\tDiscarding: the ratio would be " << ratio;
                    // std::cout << ": all visitable origins -> " << n_unvisited_origins;
                    // std::cout << ", feasible visitable origins -> " << n_unvisited_visitable_origins << std::endl;
                    feasible_reserve_out_edges.push_back(*oei); // We will use them only if it's necessary
                    continue;
                }
            }
            
            // If nothing failed, the arc is feasible
            feasible_out_edges.push_back(*oei);
        }
        
        // std::cout << "\t\tFeasible out arcs from " << graph->g[current]->id << ": " << feasible_out_edges.size() << std::endl;
        
        if(feasible_out_edges.size() == 0) {
            if(feasible_reserve_out_edges.size() == 0) {
                break;
            } else { // Use the reserves!
                feasible_out_edges = feasible_reserve_out_edges;
            }
        }
                
        std::sort(feasible_out_edges.begin(), feasible_out_edges.end(),
            [this] (const Edge& e1, const Edge& e2) {
                return (graph->g[e1]->cost > graph->g[e2]->cost);
            }
        );
        
        int random_idx = rand() % std::min(best_arcs, (int)feasible_out_edges.size());
        std::shared_ptr<Node> next = graph->g[target(feasible_out_edges[random_idx], graph->g)];
        
        // std::cout << "\t\tSelected the arc to " << next->id << std::endl;
        
        p.push_back(feasible_out_edges[random_idx]);
        load += next->demand;
        visited_nodes.push_back(next);
        current = target(feasible_out_edges[random_idx], graph->g);
        
        // std::cout << "\t\tWith the new node, we visited " << visited_nodes.size() << " nodes up to now" << std::endl;
        // std::cout << "\t\tAfter visiting the new node, the load is: " << load << std::endl;
        
        if(current == destination) {
            // std::cout << "\tWe reached the end depot!" << std::endl;
            success = true;
        }
    }
    
    return (success ? p : Path());
}

bool GreedySolver::contains(const std::vector<std::shared_ptr<Node>> visited, const int id) const {
    auto it = std::find_if(visited.begin(), visited.end(),
        [&id] (const std::shared_ptr<Node>& n) { return (n->id == id); }
    );
    
    return (it != visited.end());
}

std::vector<std::shared_ptr<Node>> GreedySolver::updated_visitable_origins(const std::vector<std::shared_ptr<Node>> visited_nodes, const int load) const {
    std::vector<std::shared_ptr<Node>> visitable_origins;
    
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(graph->g); vi != vi_end; ++vi) {
        std::shared_ptr<Node> n = graph->g[*vi];
        if(n->demand > 0) { // It's an origin
            // std::cout << "### cheching visitable origin: " << n->id << std::endl;
            if(contains(visited_nodes, n->id)) { // Already visited
                // std::cout << "### \talready visited" << std::endl;
                continue;
            }
            if(load + n->demand > std::min(graph->g[graph_bundle].ship_capacity, n->port->draught)) { // Capacity/draught
                // std::cout << "### \tcapacity violation" << std::endl;
                continue;
            }
            // std::cout << "### \tgood!" << std::endl;
            visitable_origins.push_back(n);
        }
    }
    return visitable_origins;
}

#endif