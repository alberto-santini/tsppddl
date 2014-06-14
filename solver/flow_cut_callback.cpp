#include <solver/flow_cut_callback.h>

#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/property_map/property_map.hpp>

#include <iostream>
#include <vector>

IloCplex::CallbackI* FlowCutCallback::duplicateCallback() const {
    return (new(getEnv()) FlowCutCallback(*this));
}

void FlowCutCallback::main() {
    extern unsigned int g_node_number;
    
    if(g_node_number++ % 300 == 0) {
        int n {g->g[graph_bundle].n};
        vi_t vi, vi_end;
        ei_t ei, ei_end;
    
        auto xvals = compute_x_values();
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
            std::vector<double> residual_capacity(num_edges(gr->g), 0);
            std::vector<int> colour(num_vertices(gr->g), 0);
            vertex_t source_v, sink_v;
    
            for(std::tie(vi, vi_end) = vertices(gr->g); vi != vi_end; ++vi) {
                if(gr->g[*vi].id == i) {
                    source_v = *vi;
                }
                if(gr->g[*vi].id == n+i) {
                    sink_v = *vi;
                }
            }

            double flow = boykov_kolmogorov_max_flow(gr->g,
                make_iterator_property_map(capacity.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(residual_capacity.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(reverse_edge.begin(), get(&Arc::id, gr->g)),
                make_iterator_property_map(colour.begin(), get(&Node::id, gr->g)),
                get(&Node::id, gr->g),
                source_v,
                sink_v
            );
        
            if(flow < 1 - eps) {
                // std::cout << "Violated flow: " << i << " -> " << n+i << std::endl;
            
                std::vector<int> source_nodes, sink_nodes;
            
                for(int j = 0; j < colour.size(); j++) {
                    if(j != 0 && j != 2*n+1) {
                        if(colour[j] == colour[i]) {
                            source_nodes.push_back(j);
                        } else {
                            sink_nodes.push_back(j);
                        }
                    }
                }
            
                // std::cout << "Want to force flow out of: ";
                // for(const int s : source_nodes) { std::cout << s << " "; }
                // std::cout << std::endl << "And into: ";
                // for(const int t : sink_nodes) { std::cout << t << " "; }
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

                                // std::cout << "\tAdding arc (" << i << ", " << j << ") to cut" << std::endl;
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
                    add(cut).end();
                } catch (...) {
                    cut.end();
                    throw;
                }
            }
        }
    }
}

std::vector<std::vector<double>> FlowCutCallback::compute_x_values() const {
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
    std::vector<std::vector<double>> xvals(2*n+2, std::vector<double>(2*n+2, 0));
    
    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                IloNum n = getValue(x[col_index++]);
                if(n > eps) {
                    xvals[i][j] = n;
                } else {
                    xvals[i][j] = 0;
                }
            }
         }
    }
    
    return xvals;
}