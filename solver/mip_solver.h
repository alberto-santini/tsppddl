#ifndef MIP_SOLVER_H
#define MIP_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class MipSolver {
    std::shared_ptr<const Graph> g;
    
    std::vector<Path> initial_solutions; // All initial solutions
    Path initial_solution; // Best initial solution
    
    std::string instance_name;
    
    std::vector<std::vector<int>> initial_x;
    std::vector<int> initial_y;
    std::vector<int> initial_t;
    
    void find_best_initial_solution();
    
public:
    MipSolver(const std::shared_ptr<const Graph> g, const std::vector<Path> initial_solutions, const std::string instance_name = "");
    void solve(const bool include_mtz, const bool use_valid_y_ineq = false) const;
};

#endif