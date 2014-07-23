#include <heuristics/k_opt_heuristic.h>

#include <solver/mip_solver.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>

Path KOptHeuristic::solve_with_multiple_columns() const {
    unsigned long int N {initial_solutions.size()};
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
        
    std::vector<std::vector<std::vector<int>>> sols_x;
    for(const Path& p : initial_solutions) {
        sols_x.push_back(get_x_values(p));
    }
    
    std::vector<std::vector<int>> s(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    std::vector<int> t(N, 0);
    int alpha {0};
    int beta {0};
    
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                for(int sol = 0; sol < sols_x.size(); sol++) {
                    if(sols_x[sol][i][j] == 1) {
                        s[i][j]++;
                        alpha++;
                    }
                }
            }
        }
    }
    
    for(int sol = 0; sol < N; sol++) {
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(s[i][j] == sol) {
                    t[sol]++;
                    beta = sol;
                }
            }
        }
    }
    
    MipSolver msolv {g, initial_solutions, "k-opt"};
    std::cout << "> alpha: " << alpha << ", k: " << k << ", beta: " << beta << std::endl;
    std::vector<std::vector<int>> solution_x = msolv.solve_for_k_opt(s, alpha - 12 * k * beta);
    return get_path(solution_x);
}

Path KOptHeuristic::solve() const {
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
    Path best_solution = *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
    
    std::vector<std::vector<int>> sol_x {get_x_values(best_solution)};
    
    MipSolver msolv {g, initial_solutions, "k-opt"};
    std::vector<std::vector<int>> solution_x = msolv.solve_for_k_opt(sol_x, 2 * n - k);
    return get_path(solution_x);
}

std::vector<std::vector<int>> KOptHeuristic::get_x_values(const Path& p) const {
    int n {g->g[graph_bundle].n};
    std::vector<std::vector<int>> x(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    
    for(int i = 0; i < 2 * n + 1; i++) {
        x[p.path[i]][p.path[i+1]] = 1;
    }
    
    return x;
}

Path KOptHeuristic::get_path(const std::vector<std::vector<int>>& x) const {
    cost_t c {g->cost};
    demand_t d {g->demand};
    
    int current_node {0};
    int previous_node {-1};
    int current_load {0};
    int n {g->g[graph_bundle].n};
    
    Path p;
    p.path.reserve(2 * n + 2); p.load.reserve(2 * n + 2);
    p.path.push_back(0); p.load.push_back(0);
    
    while(current_node != 2 * n + 1) {
        if(current_node == previous_node) {
            std::cerr << "I got stuck!" << std::endl;
            std::cerr << "X are:" << std::endl;
            for(int i = 0; i <= 2 * n - 1; i++) {
                for(int j = 0; j <= 2 * n - 1; j++) {
                    std::cerr << x[i][j] << " ";
                }
                std::cerr << std::endl;
            }
            throw std::runtime_error("get_path got stuck!");
        }
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(abs(x[current_node][j] - 1) < 0.01) {
                p.path.push_back(j);
                current_load += d[j];
                p.load.push_back(current_load);
                if(d[j] > 0) { p.total_load += d[j]; }
                p.total_cost += c[current_node][j];
                previous_node = current_node;
                current_node = j;
                break;
            }
        }
    }
    
    return p;
}