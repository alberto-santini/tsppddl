#ifndef K_OPT_HEURISTIC_H
#define K_OPT_HEURISTIC_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>
#include <program/program_data.h>

#include <vector>

class k_opt_heuristic {
    tsp_graph&              g;
    const program_params&   params;
    program_data&           data;
    int                     k;
    std::vector<path>       initial_solutions;
    
    std::vector<std::vector<int>> get_x_values(const path& p) const;

public:
    k_opt_heuristic(tsp_graph& g, const program_params& params, program_data& data, int k, const std::vector<path>& initial_solutions) : g{g}, params{params}, data{data}, k{k}, initial_solutions{initial_solutions} {}
    std::vector<path> solve() const;
};

#endif