#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <vector>

class HeuristicSolver {
    const Graph& g;
    
public:
    HeuristicSolver(const Graph& g) : g{g} {}
    std::vector<Path> solve() const;
};

#endif