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
    if(args.size() != 4) {
        print_usage();
        return;
    }
    
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
        bool lg_mtz {args[2] == "yes"};
        bool lg_prec {args[3] == "yes"};
        
        HeuristicSolver hsolv {g};
        heuristic_solutions = hsolv.solve();
        
        SubgradientSolver ssolv {g, heuristic_solutions, args[0], 1000};
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