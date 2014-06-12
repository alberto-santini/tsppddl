#ifndef MIP_SOLVER_H
#define MIP_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <utility>
#include <vector>

class MipSolver {
    std::shared_ptr<const Graph> g;
    
    std::vector<Path> initial_solutions; // All initial solutions
    Path initial_solution; // Best initial solution
    std::vector<std::pair<int, int>> fixable_arcs; // Arcs in common to all initial solutions
    
    std::vector<std::vector<int>> initial_x;
    std::vector<int> initial_y;
    std::vector<int> initial_t;
    
    void find_best_initial_solution();
    void find_fixable_arcs();
    
public:
    MipSolver(const std::shared_ptr<const Graph> g, const std::vector<Path> initial_solutions);
    void solve() const;
};

#endif