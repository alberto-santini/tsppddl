#include <global.h>
#include <cli/program.h>
#include <parser/parser.h>
#include <solver/heuristics/heuristic_solver.h>
#include <solver/bc/bc_solver.h>
#include <solver/subgradient/subgradient_solver.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

void Program::load(const std::string& params_filename, const std::string& instance_filename) {
    auto par = Parser(params_filename, instance_filename);
    g = std::make_unique<Graph>(par.generate_graph());
    params = std::make_unique<ProgramParams>(par.read_program_params());
}

void Program::autorun(const std::vector<std::string>& args) {
    if(args.size() != 3) {
        print_usage();
        return;
    }
    
    load(args[1], args[0]);

    if(args[2] == "branch_and_cut") {
        auto hsolv = HeuristicSolver(*g, *params);
        heuristic_solutions = hsolv.solve();
        
        auto bsolv = BcSolver(*g, *params, heuristic_solutions, args[0]);
        bsolv.solve_with_branch_and_cut();
    } else if(args[2] == "subgradient") {        
        auto hsolv = HeuristicSolver(*g, *params);
        heuristic_solutions = hsolv.solve();
        
        auto ssolv = SubgradientSolver(*g, *params, heuristic_solutions, args[0]);
        ssolv.solve();
    } else {
        print_usage();
    }
}

void Program::print_usage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t./tsppddl <instance> <params> [branch_and_cut | subgradient]" << std::endl;
    std::cout << "\t\t instance: path to the json file with the instance data" << std::endl;
    std::cout << "\t\t params: path to the json file with the program params" << std::endl;
    std::cout << "\t\t branch_and_cut: to start solving the problem with branch and cut" << std::endl;
    std::cout << "\t\t subgradient: to start solving the problem with lagrangian relaxation and the subgradient method" << std::endl;
}