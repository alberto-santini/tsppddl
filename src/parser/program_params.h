#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <parser/params/bc_params.h>
#include <parser/params/constructive_heuristics_params.h>
#include <parser/params/k_opt_params.h>
#include <parser/params/subgradient_params.h>
#include <parser/params/tabu_search_params.h>

struct program_params  {
    k_opt_params ko;
    branch_and_cut_params bc;
    tabu_search_params ts;
    tabu_search_tuning_params ts_tuning;
    constructive_heuristics_params ch;
    int cplex_threads;
    int cplex_timeout;
    
    program_params() {}
    program_params( k_opt_params ko,
                    branch_and_cut_params bc,
                    tabu_search_params ts,
                    tabu_search_tuning_params ts_tuning,
                    constructive_heuristics_params ch,
                    int cplex_threads,
                    int cplex_timeout) : 
                    ko{ko},
                    bc{bc},
                    ts{ts},
                    ts_tuning{ts_tuning},
                    ch{ch},
                    cplex_threads{cplex_threads},
                    cplex_timeout{cplex_timeout} {}
};

#endif