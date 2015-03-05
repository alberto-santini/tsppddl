#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <program/program_data.h>
#include <parser/program_params.h>

#include <vector>

class heuristic_solver {
    tsp_graph&              g;
    const program_params&   params;
    program_data&           data;
    std::vector<path>       paths;
    std::string             instance_name;
    
    std::vector<path> run_k_opt();
    std::vector<path> run_constructive(bool print_output);
    
public:
    heuristic_solver(tsp_graph& g, const program_params& params, program_data& data, std::string instance_path);
    std::vector<path> run_constructive_heuristics();
    std::vector<path> run_all_heuristics();
};

#endif