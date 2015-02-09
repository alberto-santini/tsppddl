#include <parser/parser.h>
#include <program/program.h>
#include <solver/heuristics/heuristic_solver.h>
#include <solver/bc/bc_solver.h>
#include <solver/metaheuristics/tabu/tabu_solver.h>
#include <solver/subgradient/subgradient_solver.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

program::program(const std::vector<std::string>& args) {
    if(args.size() != 3) {
        print_usage();
        return;
    }
    
    load(args[1], args[0]);

    if(args[2] == "branch_and_cut") {
        auto hsolv = heuristic_solver(g, params, data);
        auto heuristic_solutions = hsolv.solve();
        
        auto bsolv = bc_solver(g, params, data, heuristic_solutions, args[0]);
        bsolv.solve_with_branch_and_cut();
    } else if(args[2] == "subgradient") {        
        auto hsolv = heuristic_solver(g, params, data);
        auto heuristic_solutions = hsolv.solve();
        
        auto ssolv = subgradient_solver(g, params, heuristic_solutions, args[0]);
        ssolv.solve();
    } else if(args[2] == "tabu") {
        auto hsolv = heuristic_solver(g, params, data);
        auto heuristic_solutions = hsolv.solve();
        
        auto tsolv = tabu_solver(g, params, data, heuristic_solutions, args[0]);
        tsolv.solve();
    } else {
        print_usage();
    }
}

void program::load(const std::string& params_filename, const std::string& instance_filename) {
    auto par = parser(params_filename, instance_filename);
    g = std::move(par.generate_tsp_graph());
    params = std::move(par.read_program_params());
    data = program_data();
}

void program::print_usage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t./tsppddl <instance> <params> [branch_and_cut | subgradient | tabu]" << std::endl;
    std::cout << "\t\t instance: path to the json file with the instance data" << std::endl;
    std::cout << "\t\t params: path to the json file with the program params" << std::endl;
    std::cout << "\t\t branch_and_cut: to start solving the problem with branch and cut" << std::endl;
    std::cout << "\t\t subgradient: to start solving the problem with lagrangian relaxation and the subgradient method" << std::endl;
    std::cout << "\t\t tabu: to start solving the problem with the tabu search metaheuristic algorithm" << std::endl;
}