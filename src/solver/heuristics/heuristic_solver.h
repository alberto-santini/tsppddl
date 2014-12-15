#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <program/program_data.h>
#include <parser/program_params.h>

#include <vector>

class heuristic_solver {
    tsp_graph&              g;
    const program_params&   params;
    program_data&           data;
    
public:
    heuristic_solver(tsp_graph& g, const program_params& params, program_data& data) : g{g}, params{params}, data{data} {}
    std::vector<path> solve() const;
};

#endif