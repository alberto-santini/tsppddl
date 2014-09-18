#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <vector>

class HeuristicSolver {
    std::shared_ptr<const Graph> g;
    
public:
    HeuristicSolver(const std::shared_ptr<const Graph>& g) : g{g} {}
    std::vector<Path> solve() const;
};

#endif