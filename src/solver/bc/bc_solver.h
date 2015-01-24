#ifndef BC_SOLVER_H
#define BC_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>
#include <program/program_data.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <string>
#include <vector>

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

using values_matrix = std::vector<std::vector<int>>;

class bc_solver {
    tsp_graph&                      g;
    const program_params&           params;
    program_data&                   data;
    std::vector<path>               initial_solutions;
    std::string                     instance_name;
    std::string                     results_subdir;
    
    // x and y of initial_solutions
    std::vector<values_matrix>      initial_x_val;
    std::vector<values_matrix>      initial_y_val;
    
    // K-opt
    std::vector<std::vector<int>>   k_opt_lhs;
    int                             k_opt_rhs;

    static constexpr double eps = 0.00001;

    void create_results_dir(mode_t mode, const std::string& dir);
    void add_initial_solution_vals();
    path solve(bool k_opt);
    
public:
    bc_solver(tsp_graph& g, const program_params& params, program_data& data, const std::vector<path>& initial_solutions, const std::string& instance_path = "");
    void solve_with_branch_and_cut();
    path solve_for_k_opt(const path& solution, const std::vector<std::vector<int>>& lhs, int rhs);
};

#endif