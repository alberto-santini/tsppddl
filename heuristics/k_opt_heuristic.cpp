#include <heuristics/k_opt_heuristic.h>

#include <solver/bc/bc_solver.h>

#include <algorithm>
#include <iostream>
#include <limits>

std::vector<Path> KOptHeuristic::solve() const {
    auto n = g.g[graph_bundle].n;
    auto paths = std::vector<Path>();
    
    for(const auto& solution : initial_solutions) {
        auto sol_x = get_x_values(solution);
        auto msolv = BcSolver(g, params, initial_solutions, (std::to_string(k) + "-opt"));
        auto p = msolv.solve_for_k_opt(solution, sol_x, (2 * n) + 1 - k);
        
        paths.push_back(p);
    }
    
    return paths;
}

std::vector<std::vector<int>> KOptHeuristic::get_x_values(const Path& p) const {
    auto n = g.g[graph_bundle].n;
    auto x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    
    for(auto i = 0; i < 2 * n + 1; i++) {
        x[p.path[i]][p.path[i+1]] = 1;
    }
    
    return x;
}