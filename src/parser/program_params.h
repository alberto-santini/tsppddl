#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <parser/params/bc_params.h>
#include <parser/params/k_opt_params.h>
#include <parser/params/subgradient_params.h>

struct program_params  {
    k_opt_params            ko;
    subgradient_params      sg;
    branch_and_cut_params   bc;
    unsigned int            cplex_threads;
    unsigned int            cplex_timeout;
    
    program_params() {}
    program_params(k_opt_params ko, subgradient_params sg, branch_and_cut_params bc, unsigned int cplex_threads, unsigned int cplex_timeout) : ko{std::move(ko)}, sg{std::move(sg)}, bc{std::move(bc)}, cplex_threads{cplex_threads}, cplex_timeout{cplex_timeout} {}
};

#endif