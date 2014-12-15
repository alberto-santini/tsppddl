#include <heuristics/k_opt_heuristic.h>

#include <solver/bc/bc_solver.h>

#include <algorithm>
#include <iostream>
#include <limits>

std::vector<path> k_opt_heuristic::solve() const {
    auto n = g.g[graph_bundle].n;
    auto paths = std::vector<path>();
    
    for(const auto& solution : initial_solutions) {
        auto sol_x = get_x_values(solution);
        auto msolv = bc_solver(g, params, initial_solutions, (std::to_string(k) + "-opt"));
        auto p = msolv.solve_for_k_opt(solution, sol_x, (2 * n) + 1 - k);
        
        paths.push_back(p);
    }
    
    return paths;
}

std::vector<std::vector<int>> k_opt_heuristic::get_x_values(const path& p) const {
    auto n = g.g[graph_bundle].n;
    auto x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    
    for(auto i = 0; i < 2 * n + 1; i++) {
        x[p.path_v[i]][p.path_v[i+1]] = 1;
    }
    
    return x;
}