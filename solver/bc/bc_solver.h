#ifndef BC_SOLVER_H
#define BC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class BcSolver {
    std::shared_ptr<const Graph> g;
    
    std::vector<Path> initial_solutions;
    Path initial_solution; // Best one
    
    std::string instance_name;
    
    std::vector<std::vector<int>> initial_x_val;
    std::vector<int> initial_y_val;
    std::vector<int> initial_t_val;
    
    std::vector<std::vector<int>> k_opt_lhs;
    int k_opt_rhs;
    
    Path find_best_initial_solution();
    std::vector<std::vector<int>> solve(bool k_opt, bool tce) const;
    
public:
    BcSolver(const std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, const std::string instance_name = "");
    void solve_with_branch_and_cut(bool tce) const;
    std::vector<std::vector<int>> solve_for_k_opt(const std::vector<std::vector<int>>& lhs, int rhs);
};

#endif