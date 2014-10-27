#include <global.h>
#include <cli/program.h>
#include <parser/parser.h>
#include <solver/heuristics/heuristic_solver.h>
#include <solver/bc/bc_solver.h>
#include <solver/subgradient/subgradient_solver.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

void Program::load(const std::string& filename) {
    auto par = Parser(filename);
    g = std::make_unique<Graph>(par.generate_graph());
}

void Program::autorun(const std::vector<std::string>& args) {
    if(args.size() != 4) {
        print_usage();
        return;
    }
    
    auto file_name = args[0];

    load(file_name);

    if(args[1] == "branch_and_cut") {
        global::g_search_for_cuts_every_n_nodes = std::stoi(args[2]);
        
        // Two-cycles elimination
        auto tce = (args[3] == "yes");
        
        auto hsolv = HeuristicSolver(*g);
        heuristic_solutions = hsolv.solve();
        
        auto bsolv = BcSolver(*g, heuristic_solutions, args[0]);
        bsolv.solve_with_branch_and_cut(tce);
    } else if(args[1] == "subgradient") {
        auto lg_mtz = (args[2] == "yes");
        auto lg_prec = (args[3] == "yes");
        
        auto hsolv = HeuristicSolver(*g);
        heuristic_solutions = hsolv.solve();
        
        auto max_number_of_subgradient_iterations = 1000;
        
        auto ssolv = SubgradientSolver(*g, heuristic_solutions, args[0], max_number_of_subgradient_iterations);
        ssolv.solve(lg_mtz, lg_prec);
    }
}

void Program::print_usage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t./tsppddl <instance> branch_and_cut <n> <yes/no>" << std::endl;
    std::cout << "\t\t instance: path to the json file with the instance data" << std::endl;
    std::cout << "\t\t n: a number specifying how often to separate cuts (every <n> nodes)" << std::endl;
    std::cout << "\t\t yes/no: yes to add 2-cycle elimination constraints, no to skip them" << std::endl;
    std::cout << "\t./tsppddl <instance> subgradient <yes/no> <yes/no>" << std::endl;
    std::cout << "\t\t instance: path to the json file with the instance data" << std::endl;
    std::cout << "\t\t yes/no: yes to lagrangeanly relax mtz constraints, no to keep them in the model" << std::endl;
    std::cout << "\t\t yes/no: yes to lagrangeanly relax precedence constraints, no to keep them in the model" << std::endl;
}