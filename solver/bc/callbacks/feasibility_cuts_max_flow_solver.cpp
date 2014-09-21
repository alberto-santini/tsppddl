#include <global.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>

#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/property_map/property_map.hpp>

#include <chrono>
#include <ctime>
#include <ratio>

std::vector<IloRange> FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(std::shared_ptr<const Graph> g, std::shared_ptr<const Graph> gr, CallbacksHelper::solution sol, IloNumVarArray x, double eps) {
    using namespace std::chrono;
        
    int n {g->g[graph_bundle].n};
    vi_t vi, vi_end;
    ei_t ei, ei_end;

    std::vector<IloRange> cuts;

    auto xvals = sol.x;

    std::vector<double> capacity(num_edges(gr->g), 0);
    std::vector<edge_t> reverse_edge(num_edges(gr->g));

    for(std::tie(ei, ei_end) = edges(gr->g); ei != ei_end; ++ei) {
        int i {gr->g[source(*ei, gr->g)].id};
        int j {gr->g[target(*ei, gr->g)].id};
        int e {gr->g[*ei].id};

        capacity[e] = xvals[i][j];
        reverse_edge[e] = edge(target(*ei, gr->g), source(*ei, gr->g), gr->g).first;
    }
    
    std::vector<int> already_checked_cycle = std::vector<int>();

    for(int i = 1; i <= n; i++) {
        std::vector<double> residual_capacity_prec(num_edges(gr->g), 0);
        std::vector<double> residual_capacity_cycles(num_edges(gr->g), 0);
        std::vector<int> colour_prec(num_vertices(gr->g), 0);
        std::vector<int> colour_cycles(num_vertices(gr->g), 0);
        vertex_t source_v_prec, sink_v_prec, source_v_cycles, sink_v_cycles;
        
        bool skip_cycle = (std::find(already_checked_cycle.begin(), already_checked_cycle.end(), i) != already_checked_cycle.end());

        for(std::tie(vi, vi_end) = vertices(gr->g); vi != vi_end; ++vi) {
            if(gr->g[*vi].id == i) {
                source_v_prec = *vi;
            }
            if(gr->g[*vi].id == n+i) {
                source_v_cycles = *vi;
                sink_v_prec = *vi;
            }
            if(gr->g[*vi].id == 2*n+1) {
                sink_v_cycles = *vi;
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

        if(!skip_cycle) {
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
        global::g_total_time_spent_separating_cuts += time_span.count();
    
        if(flow_prec < 1 - eps) {
            std::vector<int> source_nodes, sink_nodes;
            
            for(int j = 0; j < colour_prec.size(); j++) {
                if(colour_prec[j] == colour_prec[i]) {
                    source_nodes.push_back(j);
                } else {
                    sink_nodes.push_back(j);
                }
            }
        
            IloRange cut;
            IloExpr lhs;
            IloNum rhs = 1.0;
            bool expr_init {false};

            int col_index {0};
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g->cost[ii][jj] >= 0) {
                        if( std::find(source_nodes.begin(), source_nodes.end(), ii) != source_nodes.end() &&
                            std::find(sink_nodes.begin(), sink_nodes.end(), jj) != sink_nodes.end()) {
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
                    
            cut = (lhs >= rhs);
            cuts.push_back(cut);
        }
        
        if(!skip_cycle && flow_cycles < 1 - eps) {
            std::vector<int> source_nodes, sink_nodes;
                        
            for(int j = 0; j < colour_cycles.size(); j++) {
                if(colour_cycles[j] == colour_cycles[n+i]) {
                    source_nodes.push_back(j);
                    if(j >= n+1) {
                        already_checked_cycle.push_back(j-n);
                    }
                } else {
                    sink_nodes.push_back(j);
                }
            }

            IloRange cut;
            IloExpr lhs;
            IloNum rhs = 1.0;
            bool expr_init {false};
    
            int col_index {0};
            for(int ii = 0; ii <= 2 * n + 1; ii++) {
                for(int jj = 0; jj <= 2 * n + 1; jj++) {
                    if(g->cost[ii][jj] >= 0) {
                        if( std::find(source_nodes.begin(), source_nodes.end(), ii) != source_nodes.end() &&
                            std::find(sink_nodes.begin(), sink_nodes.end(), jj) != sink_nodes.end()) {                                
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
                
            cut = (lhs >= rhs);
            cuts.push_back(cut);
        }
    }
    
    return cuts;
}