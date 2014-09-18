#include <network/graph.h>

#include <algorithm>

Graph::Graph(const demand_t& demand, const draught_t& draught, const cost_t& cost, const int capacity) : demand{demand}, draught{draught}, cost{cost} {
    int n {(static_cast<int>(demand.size()) - 2) / 2};
    
    assert(demand.size() == 2 * n + 2);
    assert(draught.size() == 2 * n + 2);
    assert(cost.size() == 2 * n + 2);
    
    g[graph_bundle] = GraphInfo {n, capacity};

    Node start_depot {0, demand[0], draught[0]}, end_depot {2*n+1, demand[2*n+1], draught[2*n+1]};
    vertex_t start_depot_v {add_vertex(g)}; g[start_depot_v] = start_depot;
    vertex_t end_depot_v {add_vertex(g)}; g[end_depot_v] = end_depot;
    
    for(int i = 1; i <= n; i++) {
        Node origin {i, demand[i], draught[i]}, destination {n + i, demand[n + i], draught[n + i]};
        vertex_t origin_v {add_vertex(g)}; g[origin_v] = origin;
        vertex_t destination_v {add_vertex(g)}; g[destination_v] = destination;
    }
        
    int arc_id {0};
    vi_t vi, vi_end, vj, vj_end;
    
    for(std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        int i {g[*vi].id};
        
        if(*vi == end_depot_v) {
            for(int j = 0; j < cost[i].size(); j++) {
                this->cost[i][j] = -1;
            }
            continue;
        }
        
        for(std::tie(vj, vj_end) = vertices(g); vj != vj_end; ++vj) {
            int j {g[*vj].id};
            
            if( (*vi == *vj) ||
                (*vj == start_depot_v) ||
                (*vi == start_depot_v && j > n) ||
                (*vj == end_depot_v && i <= n) ||
                (i == j + n) ||
                (
                    (i <= n) &&
                    (j <= n) &&
                    (g[*vi].demand + g[*vj].demand > std::min(g[*vj].draught, capacity))
                ) ||
                (
                    (i <= n) &&
                    (j > n) &&
                    (g[*vi].demand + std::abs(g[*vj].demand) > std::min(std::min(g[*vi].draught, g[*vj].draught), capacity))
                ) ||
                (
                    (i > n) &&
                    (j > n) &&
                    (std::abs(g[*vi].demand) + std::abs(g[*vj].demand) > std::min(g[*vi].draught, capacity))
                )
            ) {
                
                this->cost[i][j] = -1;
                continue;
            }
            
            this->cost[i][j] = cost[i][j];
            Arc arc {arc_id++, cost[i][j]};
            edge_t edge {add_edge(*vi, *vj, g).first}; g[edge] = arc;
        }
    }
}

Graph Graph::make_reverse_graph() const {
    Graph gr(*this);
    vi_t vi, vi_end, vj, vj_end;
    int arc_id {static_cast<int>(num_edges(gr.g))};
    
    for(std::tie(vi, vi_end) = vertices(gr.g); vi != vi_end; ++vi) {
        for(std::tie(vj, vj_end) = vertices(gr.g); vj != vj_end; ++vj) {
            if(edge(*vi, *vj, gr.g).second && !edge(*vj, *vi, gr.g).second) {
                // Take the cost from the arc in the opposite direction
                gr.cost[gr.g[*vj].id][gr.g[*vi].id] = gr.cost[gr.g[*vi].id][gr.g[*vj].id];
                Arc arc {arc_id++, gr.cost[gr.g[*vj].id][gr.g[*vi].id]};
                edge_t edge {add_edge(*vj, *vi, gr.g).first}; gr.g[edge] = arc;
            }
        }
    }
    
    return gr;
}