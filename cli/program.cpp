#include <cli/program.h>
#include <parser/parser.h>
#include <solver/heuristic_solver.h>
#include <solver/mip_solver.h>

#include <linenoise.h>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

Program::Program() {
    g = nullptr;
}

void Program::load(const std::string filename) {
    Parser par {filename};
    g = std::make_shared<Graph>(par.generate_graph());
}

void Program::autorun(const std::vector<std::string> args) {
    extern long g_search_for_cuts_every_n_nodes;
    
    std::string file_name {args[0]};
    
    if(args[1] != "lagrange" && args[1] != "lagrange_test") {    
        g_search_for_cuts_every_n_nodes = std::stoi(args[1]);
    }
        
    // 1) Load data
    load(file_name);
    
    std::cout << "***** Data loaded" << std::endl;
    
    // 2) Run the heuristics
    HeuristicSolver hsolv {g};
    heuristic_solutions = hsolv.solve();
    
    std::cout << "***** Heuristics run" << std::endl;
    
    // 3) Run CPLEX
    MipSolver msolv {g, heuristic_solutions, args[0]};
    
    if(args[1] == "lagrange") {
        std::cout << "Sbgradient method not yet implemented!" << std::endl;
    } else if(args[1] == "lagrange_test") {
        int n {g->g[graph_bundle].n};
        std::vector<std::vector<double>> mult_lambda_1(2 * n + 2, std::vector<double>(2 * n + 2, 1.0));
        std::vector<double> mult_mu_1(2 * n + 2, 1.0);
        std::vector<std::vector<double>> mult_lambda_h(2 * n + 2, std::vector<double>(2 * n + 2, 0.5));
        std::vector<double> mult_mu_h(2 * n + 2, 0.5);
        
        // 1) Run lagrange relaxation of just 1 constraint with multipliers = 1.0
        msolv.solve_with_lagrangian_relaxation_precedence(mult_mu_1);
        
        // 2) Run lagrange relaxation of just 1 constraint with multipliers = 0.5
        msolv.solve_with_lagrangian_relaxation_precedence(mult_mu_h);
        
        // 3) Run lagrange relaxation of both constraints with multipliers = 1.0
        msolv.solve_with_lagrangian_relaxation_precedence_and_cycles(mult_lambda_1, mult_mu_1);
        
        // 4) Run lagrange relaxation of both constraints with multipliers = 0.5
        msolv.solve_with_lagrangian_relaxation_precedence_and_cycles(mult_lambda_h, mult_mu_h);
        
        // 5) Run branch and cut algorithm
        msolv.solve_with_branch_and_cut();
    } else {
        msolv.solve_with_branch_and_cut();
    }
}

void Program::prompt() {
    char* line;
    
    linenoiseHistoryLoad(".history");
    
    while((line = linenoise("> ")) != NULL) {
        std::string command(line);
        
        if(command == "") {
            continue;
        }
        
        std::vector<std::string> cmd_tokens;
        boost::split(cmd_tokens, command, boost::is_any_of("\t "));
        
        if(cmd_tokens[0] == "quit" || cmd_tokens[0] == "q") {
            break;
        }
        
        if(cmd_tokens[0] == "load" || cmd_tokens[0] == "l") {
            if(cmd_tokens.size() < 2) {
                std::cout << "Not enough parameters!" << std::endl;
            } else {
                load(cmd_tokens[1]);
            }
        }
        
        if(cmd_tokens[0] == "heuristics" || cmd_tokens[0] == "h") {
            if(g != nullptr) {
                HeuristicSolver hsolv {g};
                heuristic_solutions = hsolv.solve();
                for(const Path& h : heuristic_solutions) {
                    std::cout << h.total_cost << " ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "solve" || cmd_tokens[0] == "s") {
            if(g != nullptr) {
                MipSolver msolv {g, heuristic_solutions};
                msolv.solve_with_branch_and_cut();
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
    }
}