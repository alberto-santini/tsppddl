#include <heuristics/k_opt_heuristic.h>

#include <solver/bc/bc_solver.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

Path KOptHeuristic::solve() const {
    auto n = g.g[graph_bundle].n;
    auto best_solution = *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
    auto sol_x = get_x_values(best_solution);
    auto msolv = BcSolver(g, params, initial_solutions, "k-opt");
    auto solution_x = msolv.solve_for_k_opt(sol_x, 2 * n - k);
    return get_path(solution_x);
}

std::vector<std::vector<int>> KOptHeuristic::get_x_values(const Path& p) const {
    auto n = g.g[graph_bundle].n;
    auto x = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    
    for(auto i = 0; i < 2 * n + 1; i++) {
        x[p.path[i]][p.path[i+1]] = 1;
    }
    
    return x;
}

Path KOptHeuristic::get_path(const std::vector<std::vector<int>>& x) const {
    auto current_node = 0;
    auto visited_nodes = std::vector<int>(0);
    auto current_load = 0;
    auto n = g.g[graph_bundle].n;
    
    Path p;
    p.path.reserve(2 * n + 2); p.load.reserve(2 * n + 2);
    p.path.push_back(0); p.load.push_back(0);
    visited_nodes.reserve(2 * n + 2);
        
    while(current_node != 2 * n + 1) {
        auto cycle = std::find(visited_nodes.begin(), visited_nodes.end(), current_node);
        if(cycle != visited_nodes.end()) {
            std::cerr << ">> Cycle: the path comes back to " << *cycle << std::endl;
            throw std::runtime_error("K-opt heuristic produced a path with a cycle!");
        }
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(std::abs(x[current_node][j] - 1) < 0.0001) {
                p.path.push_back(j);
                current_load += g.demand[j];
                p.load.push_back(current_load);
                if(g.demand[j] > 0) { p.total_load += g.demand[j]; }
                p.total_cost += g.cost[current_node][j];
                visited_nodes.push_back(current_node);
                current_node = j;
                break;
            }
        }
    }
    
    if(p.path.size() != (size_t)(2 * n + 2)) {
        std::cerr << "Path length: " << p.path.size() << " - expected: " << (2*n+2) << std::endl;
        std::cerr << "Path: ";
        for(auto i = 0u; i < p.path.size(); i++) {
            std::cerr << p.path[i] << " ";
        }
        std::cerr << std::endl;
        std::cerr << "X: " << std::endl;
        for(auto i = 0; i <= 2*n+1; i++) {
            for(auto j = 0; j <= 2*n+1; j++) {
                if(x[i][j] > 0.0) {
                    std::cerr << "x[" << i << "][" << j << "] = " << x[i][j] << std::endl;
                }
            }
        }
        throw std::runtime_error("K-opt heuristic produced a short path!");
    }
    
    return p;
}