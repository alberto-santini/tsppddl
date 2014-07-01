#include <solver/flow_cut_callback.h>

#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/property_map/property_map.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>
#include <vector>

IloCplex::CallbackI* FlowCutCallback::duplicateCallback() const {
    return (new(getEnv()) FlowCutCallback(*this));
}

void FlowCutCallback::main() {
    using namespace std::chrono;
    
    extern long g_node_number;
    extern double g_total_time_spent_separating_cuts;
    extern long g_search_for_cuts_every_n_nodes;
    extern long g_total_bb_nodes_explored;
    extern long g_total_number_of_cuts_added;
    extern long g_number_of_cuts_added_at_root;
    
    g_total_bb_nodes_explored++;
    
    auto integer_and_vals = compute_x_values();
    bool is_integer_solution {integer_and_vals.first};
    
    if(g_node_number++ % g_search_for_cuts_every_n_nodes == 0) {
        int n {g->g[graph_bundle].n};
        vi_t vi, vi_end;
        ei_t ei, ei_end;
    
        auto xvals = integer_and_vals.second;
    
        std::vector<double> capacity(num_edges(gr->g), 0);
        std::vector<edge_t> reverse_edge(num_edges(gr->g));
    
        for(std::tie(ei, ei_end) = edges(gr->g); ei != ei_end; ++ei) {
            int i {gr->g[source(*ei, gr->g)].id};
            int j {gr->g[target(*ei, gr->g)].id};
            int e {gr->g[*ei].id};
    
            capacity[e] = xvals[i][j];
            reverse_edge[e] = edge(target(*ei, gr->g), source(*ei, gr->g), gr->g).first;
        }
    
        for(int i = 1; i <= n; i++) {
            std::vector<double> residual_capacity_prec(num_edges(gr->g), 0);
            std::vector<double> residual_capacity_cycles(num_edges(gr->g), 0);
            std::vector<int> colour_prec(num_vertices(gr->g), 0);
            std::vector<int> colour_cycles(num_vertices(gr->g), 0);
            vertex_t source_v_prec, sink_v_prec, source_v_cycles, sink_v_cycles;
    
            for(std::tie(vi, vi_end) = vertices(gr->g); vi != vi_end; ++vi) {
                if(gr->g[*vi].id == i) {
                    source_v_prec = *vi;
                }
                if(gr->g[*vi].id == n+i) {
                    sink_v_prec = *vi;
                }
            }

            high_resolution_clock::time_point t_start {high_resolution_clock::now()};
            double flow_prec, flow_cycles;

            flow_prec = boykov_kolmogorov_max_flow(gr->g,
                make_iterator_property_map(capacity.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(residual_capacity_prec.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(reverse_edge.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(colour_prec.begin(), get(&Node::id, gr->g)),
                get(&Node::id, gr->g),
                source_v_prec,
                sink_v_prec
            );
                
            if(separate_all) {
                for(std::tie(vi, vi_end) = vertices(gr->g); vi != vi_end; ++vi) {
                    if(gr->g[*vi].id == n+i) {
                        source_v_cycles = *vi;
                    }
                    if(gr->g[*vi].id == 2*n+1) {
                        sink_v_cycles = *vi;
                    }
                }
                
                flow_cycles = boykov_kolmogorov_max_flow(gr->g,
                    make_iterator_property_map(capacity.begin(), get(&Arc::id, gr->g)),
                    make_iterator_property_map(residual_capacity_cycles.begin(), get(&Arc::id, gr->g)),
                    make_iterator_property_map(reverse_edge.begin(), get(&Arc::id, gr->g)),
                    make_iterator_property_map(colour_cycles.begin(), get(&Node::id, gr->g)),
                    get(&Node::id, gr->g),
                    source_v_cycles,
                    sink_v_cycles
                ); 
            }
                
            high_resolution_clock::time_point t_end {high_resolution_clock::now()};
            duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};
            g_total_time_spent_separating_cuts += time_span.count();
        
            if(flow_prec < 1 - eps) {
                std::vector<int> source_nodes, sink_nodes;
            
                for(int j = 0; j < colour_prec.size(); j++) {
                    if(j != 0 && j != 2*n+1) {
                        if(colour_prec[j] == colour_prec[i]) {
                            source_nodes.push_back(j);
                        } else {
                            sink_nodes.push_back(j);
                        }
                    }
                }
                
                // std::cout << "PREC CUT: ";
                // for(int ii : source_nodes) { std::cout << ii << " "; }
                // std::cout << " | ";
                // for(int jj : sink_nodes) { std::cout << jj << " "; }
                // std::cout << std::endl;
            
                IloRange cut;
                IloExpr lhs;
                IloNum rhs = 1.0;
                bool expr_init {false};
            
                int col_index {0};
                for(int i = 0; i <= 2 * n + 1; i++) {
                    for(int j = 0; j <= 2 * n + 1; j++) {
                        if(g->cost[i][j] >= 0) {
                            if( std::find(source_nodes.begin(), source_nodes.end(), i) != source_nodes.end() &&
                                std::find(sink_nodes.begin(), sink_nodes.end(), j) != sink_nodes.end()) {
                                if(!expr_init) {
                                    lhs = x[col_index];
                                    expr_init = true;
                                } else {
                                    lhs += x[col_index];
                                }
                            }
                            col_index++;
                        }
                    }
                }
                        
                try {
                    cut = (lhs >= rhs);
                    add(cut, IloCplex::UseCutFilter).end();
                    g_total_number_of_cuts_added++;
                    if(g_total_bb_nodes_explored == 1) {
                        g_number_of_cuts_added_at_root++;
                    }
                } catch (...) {
                    cut.end();
                    throw;
                }
            }
            
            if(separate_all && flow_cycles < 1 - eps) {
                std::vector<int> source_nodes, sink_nodes;
        
                for(int j = 0; j < colour_cycles.size(); j++) {
                    if(colour_cycles[j] == colour_cycles[n+i]) {
                        source_nodes.push_back(j);
                    } else {
                        sink_nodes.push_back(j);
                    }
                }
                
                // std::cout << "CYCL CUT: ";
                // for(int ii : source_nodes) { std::cout << ii << " "; }
                // std::cout << " | ";
                // for(int jj : sink_nodes) { std::cout << jj << " "; }
                // std::cout << std::endl;
        
                IloRange cut;
                IloExpr lhs;
                IloNum rhs = 1.0;
                bool expr_init {false};
        
                int col_index {0};
                for(int i = 0; i <= 2 * n + 1; i++) {
                    for(int j = 0; j <= 2 * n + 1; j++) {
                        if(g->cost[i][j] >= 0) {
                            if( std::find(source_nodes.begin(), source_nodes.end(), i) != source_nodes.end() &&
                                std::find(sink_nodes.begin(), sink_nodes.end(), j) != sink_nodes.end()) {
                                if(!expr_init) {
                                    lhs = x[col_index];
                                    expr_init = true;
                                } else {
                                    lhs += x[col_index];
                                }
                            }
                            col_index++;
                        }
                    }
                }
                    
                try {
                    cut = (lhs >= rhs);
                    add(cut, IloCplex::UseCutFilter).end();
                    g_total_number_of_cuts_added++;
                    if(g_total_bb_nodes_explored == 1) {
                        g_number_of_cuts_added_at_root++;
                    }
                } catch (...) {
                    cut.end();
                    throw;
                }
            }
        }
    }
}

std::pair<bool, std::vector<std::vector<double>>> FlowCutCallback::compute_x_values() const {
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
    std::vector<std::vector<double>> xvals(2*n+2, std::vector<double>(2*n+2, 0));
    bool is_integer {true};
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                IloNum n = getValue(x[col_index++]);
                
                if(n > eps) {
                    if(n < 1 - eps) {
                        is_integer = false;
                    }
                    xvals[i][j] = n;
                } else {
                    xvals[i][j] = 0;
                }
            }
         }
    }
    
    return std::make_pair(is_integer, xvals);
}