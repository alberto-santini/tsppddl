#ifndef HEURISTIC_HELPER_CPP
#define HEURISTIC_HELPER_CPP 

#include <solver/heuristic_helper.h>

HeuristicHelper::HeuristicHelper(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph) : data(data), graph(graph) {
    ship_capacity = data->capacity;
    n = data->num_requests;
    distance = std::vector<std::vector<double>>(2 * n + 2, std::vector<double>(2 * n + 2));
    draught = std::vector<int>(2 * n + 2);
    demand = std::vector<int>(2 * n + 2);
    
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(graph->g); vi != vi_end; ++vi) {
        std::shared_ptr<Node> n = graph->g[*vi];
        draught[n->id] = n->port->draught;
        demand[n->id] = n->demand;
        
        vit vi2, vi2_end;
        for(std::tie(vi2, vi2_end) = vertices(graph->g); vi2 != vi2_end; ++vi2) {
            std::shared_ptr<Node> n2 = graph->g[*vi2];
            distance[n->id][n2->id] = data->distances[n->port->id][n2->port->id];
        }
    }
}

double HeuristicHelper::test_path_cost(const std::vector<int>& path) const {
    double cost = 0; double load = 0;
    for(int i = 0; i < path.size() - 1; i++) {
        eit ei, ei_end;
        for(std::tie(ei, ei_end) = edges(graph->g); ei != ei_end; ++ei) {
            std::shared_ptr<Node> source_n = graph->g[source(*ei, graph->g)];
            std::shared_ptr<Node> target_n = graph->g[target(*ei, graph->g)];
            if(source_n->id == path[i] && target_n->id == path[i+1]) {
                cost += graph->g[*ei]->cost;
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated upon arriving at " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
                load += target_n->demand;
                if(load > ship_capacity) {
                    std::cout << "Ship capacity violated after servicing " << target_n->id << " (" << load << " vs. capacity " << ship_capacity << ")" << std::endl;
                }
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated after servicing " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
            }
        }
    }    
    return cost;
}

std::pair<bool, placement> HeuristicHelper::new_path_if_feasible(const int& x, const int& y, const int& i, const double& length, const int& load, const double& best_metric, const std::vector<int>& path, const std::vector<int>& partial_load) const {
    std::vector<int> new_path(path.size() + 2);
    std::vector<int> new_partial_load(partial_load.size() + 2);
    double new_length = 0;
    int new_load = 0;
    double new_metric = 0;
    
    if(x != y) {
        new_length =    length -
                        distance[path[x-1]][path[x]] -
                        distance[path[y-1]][path[y]] +
                        distance[path[x-1]][i] +
                        distance[i][path[x]] +
                        distance[path[y-1]][n+i] +
                        distance[n+i][path[y]];
    } else {
        // Insert i and n+i consecutively
        new_length =    length -
                        distance[path[x-1]][path[x]] +
                        distance[path[x-1]][i] +
                        distance[i][n+i] +
                        distance[n+i][path[x]];
    }
    new_load = load + demand[i];
    new_metric = new_load / new_length;
        
    if(new_metric <= best_metric) {
        // Can't improve
        return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
    }
    
    if(x != y) {
        for(int j = 0; j <= x-1; j++) {
            new_path[j] = path[j];
            new_partial_load[j] = partial_load[j];
        }
    
        new_path[x] = i;
        new_partial_load[x] = new_partial_load[x-1] + demand[i];
    
        if(new_partial_load[x] > std::min(std::min(draught[i], draught[path[x]]), ship_capacity)) {
            return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
        }
    
        for(int j = x + 1; j <= y; j++) {
            new_path[j] = path[j-1];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j]];
        
            int next_port_draught = (j < y ? draught[path[j]] : draught[n+i]);
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
            }
        }
    
        new_path[y+1] = n+i;
        new_partial_load[y+1] = new_partial_load[y] + demand[n+i];
    
        if(new_partial_load[y+1] > std::min(std::min(draught[n+i], draught[path[y]]), ship_capacity)) {
            return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
        }
    
        for(int j = y + 2; j < path.size() + 2; j++) {
            new_path[j] = path[j-2];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j]];
                
            int next_port_draught = (j <= path.size() ? draught[path[j-1]] : std::numeric_limits<int>::max());
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
            }
        }
    } else {
        for(int j = 0; j <= x-1; j++) {
            new_path[j] = path[j];
            new_partial_load[j] = partial_load[j];
        }
        
        new_path[x] = i;
        new_partial_load[x] = new_partial_load[x-1] + demand[i];
        
        if(new_partial_load[x] > std::min(std::min(draught[i], draught[n+i]), ship_capacity)) {
            return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
        }
        
        new_path[x+1] = n+i;
        new_partial_load[x+1] = new_partial_load[x] + demand[n+i];
        
        if(new_partial_load[x] > std::min(std::min(draught[n+i], draught[path[x]]), ship_capacity)) {
            return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
        }
        
        for(int j = x + 2; j < path.size() + 2; j++) {
            new_path[j] = path[j-2];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j]];
            
            int next_port_draught = (j <= path.size() ? draught[path[j-1]] : std::numeric_limits<int>::max());
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                return std::make_pair(false, std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1, -1, -1));
            }
        }
    }
    
    return std::make_pair(true, std::make_tuple(new_path, new_partial_load, new_length, new_load, new_metric));
}

#endif