#include <program/program_data.h>

#include <algorithm>

void program_data::reset_times_and_cuts() {
    auto new_data = program_data();
    new_data.time_spent_by_constructive_heuristics = time_spent_by_constructive_heuristics;
    new_data.time_spent_by_k_opt_heuristics = time_spent_by_k_opt_heuristics;
    new_data.time_spent_by_tabu_search = time_spent_by_tabu_search;
    
    std::swap(*this, new_data);
}

void program_data::reset_for_new_branch_and_cut() {
    auto new_data = program_data();
    new_data.time_spent_by_constructive_heuristics = time_spent_by_constructive_heuristics;
    new_data.time_spent_by_k_opt_heuristics = time_spent_by_k_opt_heuristics;
    new_data.time_spent_by_tabu_search = time_spent_by_tabu_search;
    
    new_data.n_constructive_solutions = n_constructive_solutions;
    new_data.best_constructive_solution = best_constructive_solution;
    new_data.best_tabu_solution = best_tabu_solution;
    
    std::swap(*this, new_data);
}