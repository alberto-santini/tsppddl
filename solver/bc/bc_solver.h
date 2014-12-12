#ifndef BC_SOLVER_H
#define BC_SOLVER_H

#include <network/graph.h>
#include <network/path.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <string>
#include <vector>

using values_matrix = std::vector<std::vector<int>>;

class BcSolver {
    const Graph& g;
    const ProgramParams& params;
    
    std::vector<Path> initial_solutions;
    
    std::string instance_name;
    
    // x and y of initial_solutions
    std::vector<values_matrix> initial_x_val;
    std::vector<values_matrix> initial_y_val;
    
    std::vector<std::vector<int>> k_opt_lhs;
    int k_opt_rhs;

    void add_initial_solution_vals();
    Path solve(bool k_opt) const;
    
public:
    BcSolver(const Graph& g, const ProgramParams& params, const std::vector<Path>& initial_solutions, const std::string& instance_path = "");
    void solve_with_branch_and_cut() const;
    Path solve_for_k_opt(const Path& solution, const std::vector<std::vector<int>>& lhs, int rhs);
};

#endif