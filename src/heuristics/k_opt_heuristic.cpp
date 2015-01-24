#include <heuristics/k_opt_heuristic.h>
#include <solver/bc/bc_solver.h>

std::vector<path> k_opt_heuristic::solve() const {
    auto n = g.g[graph_bundle].n;
    auto paths = std::vector<path>();
    
    for(const auto& solution : initial_solutions) {
        auto sol_x = solution.get_x_values(n);
        auto msolv = bc_solver(g, params, data, initial_solutions, (std::to_string(k) + "-opt"));
        auto p = msolv.solve_for_k_opt(solution, sol_x, (2 * n) + 1 - k);
        
        paths.push_back(p);
    }
    
    return paths;
}