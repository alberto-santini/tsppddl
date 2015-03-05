#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <parser/params/bc_params.h>
#include <parser/params/constructive_heuristics_params.h>
#include <parser/params/k_opt_params.h>
#include <parser/params/subgradient_params.h>
#include <parser/params/tabu_search_params.h>

struct program_params  {
    k_opt_params ko;
    subgradient_params sg;
    branch_and_cut_params bc;
    tabu_search_params ts;
    constructive_heuristics_params ch;
    unsigned int cplex_threads;
    unsigned int cplex_timeout;
    
    program_params() {}
    program_params(k_opt_params ko, subgradient_params sg, branch_and_cut_params bc, tabu_search_params ts, constructive_heuristics_params ch, unsigned int cplex_threads, unsigned int cplex_timeout) : ko{std::move(ko)}, sg{std::move(sg)}, bc{std::move(bc)}, ts{std::move(ts)}, ch{std::move(ch)}, cplex_threads{cplex_threads}, cplex_timeout{cplex_timeout} {}
};

#endif