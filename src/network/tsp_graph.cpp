#include <network/tsp_graph.h>

#include <algorithm>

tsp_graph::tsp_graph(const demand_t& demand, const draught_t& draught, const cost_t& cost, int capacity, std::string instance_path) : demand{demand}, draught{draught}, cost{cost} {
    auto n = (size_t)((demand.size() - 2) / 2);
    
    assert(demand.size() == (2 * n + 2));
    assert(draught.size() == (2 * n + 2));
    assert(cost.size() == (2 * n + 2));
    
    g[graph_bundle] = graph_info(n, capacity, instance_path);

    auto start_depot = node(0, demand[0], draught[0]);
    auto end_depot = node(2*n+1, demand[2*n+1], draught[2*n+1]);
    auto start_depot_v = add_vertex(g);
    g[start_depot_v] = start_depot;
    auto end_depot_v = add_vertex(g);
    g[end_depot_v] = end_depot;
    
    for(auto i = 1u; i <= n; i++) {
        auto origin = node(i, demand[i], draught[i]);
        auto destination = node(n + i, demand[n + i], draught[n + i]);
        auto origin_v = add_vertex(g); g[origin_v] = origin;
        auto destination_v = add_vertex(g); g[destination_v] = destination;
    }
        
    auto arc_id = 0;
    vi_t vi, vi_end, vj, vj_end;
    
    for(std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        auto i = g[*vi].id;
        
        if(*vi == end_depot_v) {
            for(auto j = 0u; j < cost[i].size(); j++) {
                this->cost[i][j] = -1;
            }
            continue;
        }
        
        for(std::tie(vj, vj_end) = vertices(g); vj != vj_end; ++vj) {
            auto j = g[*vj].id;
            
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
                    (j != i + n) &&
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
            auto new_arc = arc(arc_id++, cost[i][j]);
            auto edge = add_edge(*vi, *vj, g).first;
            g[edge] = new_arc;
        }
    }
    
    populate_list_of_infeasible_3_paths();
}

void tsp_graph::populate_list_of_infeasible_3_paths() {
    auto n = g[graph_bundle].n;
    
    for(auto i = 1; i <= 2*n; i++) {
        for(auto j = 1; j <= 2*n; j++) {
            for(auto k = 1; k <= 2*n; k++) {
                if(cost[i][j] >= 0 && cost[j][k] >= 0) {
                    infeas_list[{i, j, k}] = is_path_eliminable(i, j, k);
                }
            }
        }
    }
}

// Check if (i,j) -> (j,k) can be a subpath of a feasible path
// It assumes that (i,j) and (j,k) both exist
// It assumes that 1 <= i,j,k <= 2n
// It returns true if any path containing that subpath should be eliminated
bool tsp_graph::is_path_eliminable(int i, int j, int k) const {
    auto n = g[graph_bundle].n;
    auto Q = g[graph_bundle].capacity;
        
    if(i == n + k) {
        return true;
    }
    
    auto min3 = [] (auto x, auto y, auto z) { return std::min(x, std::min(y,z)); };
    
    if(i <= n) {
        if(j <= n) {
            if(k <= n) {
                return (demand.at(i) + demand.at(j) + demand.at(k) > std::min(Q, draught.at(k)));
            } else {
                return (    k != n+i && k != n+j && (
                                demand.at(i) + demand.at(j) + demand.at(k-n) > min3(Q, draught.at(j), draught.at(k)) ||
                                demand.at(i) + demand.at(k-n) > std::min(Q, draught.at(k))
                            )
                       );
            }
        } else {
            if(k <= n) {
                return (j != n+i && demand.at(i) + demand.at(k) > std::min(Q, draught.at(k)));
            } else {
                return (    j != n+i && k != n+i && (
                                demand.at(i) + demand.at(j-n) + demand.at(k-n) > min3(Q, draught.at(i), draught.at(j)) ||
                                demand.at(i) + demand.at(k-n) > std::min(Q, draught.at(k))
                            )
                       );
            }
        }
    } else {
        if(j <= n) {
            if(k <= n) {
                return false;
            } else {
                return (k != n+j && demand.at(i-n) + demand.at(k-n) > std::min(Q, draught.at(i)));
            }
        } else {
            if(k <= n) {
                return false;
            } else {
                return (demand.at(i-n) + demand.at(j-n) + demand.at(k-n) > std::min(Q, draught.at(i)));
            }
        }
    }
}

tsp_graph tsp_graph::make_reverse_tsp_graph() const {
    auto gr = tsp_graph(*this);
    auto arc_id = num_edges(gr.g);
    vi_t vi, vi_end, vj, vj_end;
    
    for(std::tie(vi, vi_end) = vertices(gr.g); vi != vi_end; ++vi) {
        for(std::tie(vj, vj_end) = vertices(gr.g); vj != vj_end; ++vj) {
            if(edge(*vi, *vj, gr.g).second && !edge(*vj, *vi, gr.g).second) {
                // Take the cost from the arc in the opposite direction
                gr.cost[gr.g[*vj].id][gr.g[*vi].id] = gr.cost[gr.g[*vi].id][gr.g[*vj].id];
                auto new_arc = arc(arc_id++, gr.cost[gr.g[*vj].id][gr.g[*vi].id]);
                auto edge = add_edge(*vj, *vi, gr.g).first;
                gr.g[edge] = new_arc;
            }
        }
    }
    
    return gr;
}