#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <vector>

class HeuristicSolver {
    const Graph& g;
    const ProgramParams& params;
    
public:
    HeuristicSolver(const Graph& g, const ProgramParams& params) : g{g}, params{params} {}
    std::vector<Path> solve() const;
};

#endif