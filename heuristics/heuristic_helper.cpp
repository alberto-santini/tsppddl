#ifndef HEURISTIC_HELPER_H
#define HEURISTIC_HELPER_H

#include <heuristics/heuristic_helper.h>

int HeuristicHelper::test_path_cost(const GenericPath& p, const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph) {
    int cost = 0; int load = 0;
    for(int i = 0; i < p.path.size() - 1; i++) {
        eit ei, ei_end;
        bool edge_found = false;
        for(std::tie(ei, ei_end) = edges(graph->g); ei != ei_end; ++ei) {
            std::shared_ptr<Node> source_n = graph->g[source(*ei, graph->g)];
            std::shared_ptr<Node> target_n = graph->g[target(*ei, graph->g)];
            if(source_n->id == p.path[i] && target_n->id == p.path[i+1]) {
                edge_found = true;
                cost += graph->g[*ei]->cost;
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated upon arriving at " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
                load += target_n->demand;
                if(load > data->capacity) {
                    std::cout << "Ship capacity violated after servicing " << target_n->id << " (" << load << " vs. capacity " << data->capacity << ")" << std::endl;
                }
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated after servicing " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
            }
        }
        if(!edge_found) {
            std::cout << "Could not find edge " << p.path[i] << " -> " << p.path[i+1] << std::endl;
        }
    }    
    return cost;
}

#endif