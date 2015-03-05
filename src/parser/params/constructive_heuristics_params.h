#ifndef CONSTRUCTIVE_HEURISTICS_PARAMS_H
#define CONSTRUCTIVE_HEURISTICS_PARAMS_H

#include <string>

struct constructive_heuristics_params {
    bool print_solutions;
    std::string results_dir;
    std::string solutions_dir;
    
    constructive_heuristics_params() {}
    constructive_heuristics_params(bool print_solutions, std::string results_dir, std::string solutions_dir) : print_solutions{print_solutions}, results_dir{results_dir}, solutions_dir{solutions_dir} {}
};

#endif