#ifndef BC_SOLVER_H
#define BC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <string>
#include <utility>
#include <vector>

class BcSolver {
    const Graph& g;
    const ProgramParams& params;
    
    std::vector<Path> initial_solutions;
    Path initial_solution; // Best one
    
    std::string instance_name;
    
    std::vector<std::vector<int>> initial_x_val;
    std::vector<std::vector<int>> initial_y_val;
    
    std::vector<std::vector<int>> k_opt_lhs;
    int k_opt_rhs;
    
    Path find_best_initial_solution();
    std::vector<std::vector<int>> solve(bool k_opt) const;
    
public:
    BcSolver(const Graph& g, const ProgramParams& params, const std::vector<Path>& initial_solutions, const std::string& instance_path = "");
    void solve_with_branch_and_cut() const;
    std::vector<std::vector<int>> solve_for_k_opt(const std::vector<std::vector<int>>& lhs, int rhs);
};

#endif