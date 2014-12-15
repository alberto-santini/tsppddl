#include <program/program_data.h>

#include <algorithm>

void program_data::reset_times_and_cuts() {
    auto new_data = program_data();
    new_data.time_spent_by_constructive_heuristics = time_spent_by_constructive_heuristics;
    new_data.time_spent_by_k_opt_heuristics = time_spent_by_k_opt_heuristics;
    
    std::swap(*this, new_data); 
}