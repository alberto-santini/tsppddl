#include <network/graph_writer.h>
#include <solver/bc/callbacks/print_relaxation_graph_callback.h>

#include <iostream>
#include <string>

IloCplex::CallbackI* print_relaxation_graph_callback::duplicateCallback() const {
    return (new(getEnv()) print_relaxation_graph_callback(*this));
}

void print_relaxation_graph_callback::main() {
    auto node_number = getNnodes();
    
    // if(node_number == 0) {
        std::cerr << "Printing graphviz! Node: " << node_number << std::endl;
        
        auto n = g.g[graph_bundle].n;
        auto solution_x = std::vector<std::vector<double>>(2 * n + 2, std::vector<double>(2 * n + 2, 0));
        auto solution_y = std::vector<std::vector<double>>(2 * n + 2, std::vector<double>(2 * n + 2, 0));
        
        auto col_index = 0;
        for(auto i = 0; i <= 2 * n + 1; i++) {
            for(auto j = 0; j <= 2 * n + 1; j++) {
                if(g.cost[i][j] >= 0) {
                    auto xv = getValue(x[col_index]);
                    auto yv = getValue(y[col_index]);
                    
                    if(xv > 0.0001) {
                        solution_x[i][j] = xv;
                    }
                    if(yv > 0.0001) {
                        solution_y[i][j] = yv;
                    }
                    
                    col_index++;
                }
             }
        }
    
        auto gw = graph_writer(g, std::move(solution_x), std::move(solution_y));
        gw.write("graphs/" + instance_name + "." + std::to_string(node_number));
    // }
}