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
    
    std::vector<std::vector<double>> mult_lambda;
    std::vector<double> mult_mu;
    
    std::vector<std::vector<int>> k_opt_lhs;
    int k_opt_rhs;
    
    void find_best_initial_solution();
    std::vector<std::vector<int>> solve(const bool include_mtz, const bool use_lagrange_cycles, const bool use_lagrange_precedence, const bool use_valid_y_ineq, const bool k_opt) const;
    
public:
    MipSolver(const std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, const std::string& instance_name = "");
    void solve_with_mtz(const bool use_valid_y_ineq) const;
    void solve_with_lagrangian_relaxation_precedence(const std::vector<double>& mult_mu, const bool use_valid_y_ineq);
    void solve_with_lagrangian_relaxation_precedence_and_cycles(const std::vector<std::vector<double>>& mult_lambda, const std::vector<double>& mult_mu, const bool use_valid_y_ineq);
    void solve_with_branch_and_cut(const bool use_valid_y_ineq) const;
    std::vector<std::vector<int>> solve_for_k_opt(const std::vector<std::vector<int>>& lhs, const int& rhs);
};

#endif