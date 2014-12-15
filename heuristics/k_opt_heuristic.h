#ifndef K_OPT_HEURISTIC_H
#define K_OPT_HEURISTIC_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <vector>

class k_opt_heuristic {
    tsp_graph& g;
    const program_params& params;
    int k;
    std::vector<path> initial_solutions;
    
    std::vector<std::vector<int>> get_x_values(const path& p) const;
    
public:
    k_opt_heuristic(tsp_graph& g, const program_params& params, int k, const std::vector<path>& initial_solutions) : g{g}, params{params}, k{k}, initial_solutions{initial_solutions} {}
    std::vector<path> solve() const;
};

#endif