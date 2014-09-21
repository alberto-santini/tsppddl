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
    Parser par {filename};
    g = std::make_shared<Graph>(par.generate_graph());
}

void Program::autorun(const std::vector<std::string>& args) {
    std::string file_name {args[0]};

    load(file_name);

    if(args[1] == "branch_and_cut") {
        global::g_search_for_cuts_every_n_nodes = std::stoi(args[2]);
        bool tce {args[3] == "yes"};
        
        HeuristicSolver hsolv {g};
        heuristic_solutions = hsolv.solve();
        
        BcSolver bsolv {g, heuristic_solutions, args[0]};
        bsolv.solve_with_branch_and_cut(tce);
    } else if(args[1] == "subgradient") {
        HeuristicSolver hsolv {g};
        heuristic_solutions = hsolv.solve();
        
        SubgradientSolver ssolv {g, heuristic_solutions, args[0], 1000};
        ssolv.solve();
    } else {
        std::cout << "Possible args: " << std::endl;
        std::cout << "tsppddl <instance> branch_and_cut <cut_every_n_nodes> <2-cycles_elimination>" << std::endl;
        std::cout << "tsppddl <instance> subgradient" << std::endl;
    }
}