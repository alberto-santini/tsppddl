#ifndef RAW_DATA_CPP
#define RAW_DATA_CPP

#include <heuristics/raw_data.h>

RawData::RawData(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph) {
    n = data->num_requests;
    Q = data->capacity;  
    d = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2));
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
            d[n->id][n2->id] = data->distances[n->port->id][n2->port->id];
        }
    }
}

#endif