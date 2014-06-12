#include <cli/program.h>
#include <parser/parser.h>
#include <solver/heuristic_solver.h>
#include <solver/mip_solver.h>

#include <linenoise.h>
#include <boost/algorithm/string.hpp>

#include <iostream>

Program::Program() {
    g = nullptr;
}

void Program::load(const std::string filename) {
    Parser par {filename};
    g = std::make_shared<Graph>(par.generate_graph());
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