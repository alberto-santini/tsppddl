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
    extern double g_total_time_spent_by_heuristics;
    extern double g_total_time_spent_separating_cuts;
    extern long g_total_bb_nodes_explored;
    extern long g_total_number_of_cuts_added;
    extern long g_number_of_cuts_added_at_root;
    extern double g_time_spent_at_root;
    extern double g_ub_at_root;
    extern double g_lb_at_root;
    extern double g_ub;
    extern double g_lb;
    extern double g_total_cplex_time;
    extern int g_search_for_cuts_every_n_nodes;
    
    std::string file_name {args[0]};
    g_search_for_cuts_every_n_nodes = std::stoi(args[1]);
    
    // 1) Load data
    load(file_name);
    
    // 2) Run the heuristics
    HeuristicSolver hsolv {g};
    heuristic_solutions = hsolv.solve();
    
    // 3) Run CPLEX
    MipSolver msolv {g, heuristic_solutions};
    msolv.solve();

    std::ofstream results_file;
    results_file.open("results.txt", std::ios::out | std::ios::app);
    
    // 4) Print statistics
    results_file << args[0] << "\t";
    results_file << g_search_for_cuts_every_n_nodes << "\t";
    results_file << std::setw(12);
    results_file << g_total_cplex_time << "\t";
    results_file << g_total_time_spent_by_heuristics << "\t";
    results_file << g_total_time_spent_separating_cuts << "\t";
    results_file << g_time_spent_at_root << "\t";
    results_file << g_ub << "\t";
    results_file << g_lb << "\t";
    results_file << g_ub_at_root << "\t";
    results_file << g_lb_at_root << "\t";
    results_file << g_total_number_of_cuts_added << "\t";
    results_file << g_number_of_cuts_added_at_root << "\t";
    results_file << g_total_bb_nodes_explored << std::endl;
    
    results_file.close();
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
                msolv.solve();
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
    }
}