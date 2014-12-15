#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <vector>

class heuristic_solver {
    tsp_graph& g;
    const program_params& params;
    
public:
    heuristic_solver(tsp_graph& g, const program_params& params) : g{g}, params{params} {}
    std::vector<path> solve() const;
};

#endif