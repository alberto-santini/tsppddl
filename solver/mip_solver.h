#ifndef MIP_SOLVER_H
#define MIP_SOLVER_H

#include <memory>
#include <vector>

#include <heuristics/generic_path.h>
#include <parser/data.h>
#include <network/graph.h>

class MipSolver {
    int n;
    std::shared_ptr<Graph> graph;
    GenericPath initial_solution;
    std::vector<std::vector<int>> initial_x;
    std::vector<int> initial_y;
    std::vector<int> initial_t;
    
public:
    MipSolver(std::shared_ptr<Graph> graph, const GenericPath initial_solution);
    void solve() const;
};

#endif