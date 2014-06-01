#ifndef PROGRAM_CPP
#define PROGRAM_CPP

#include <iostream>

#include <linenoise.h>
#include <boost/algorithm/string.hpp>

#include <cli/program.h>
#include <solver/mip_solver.h>
#include <solver/labelling_solver.h>

Program::Program() {
    data = nullptr;
    graph = nullptr;
    hs = nullptr;
    initial_solution = GenericPath();
}

void Program::load(std::string instance_file) {
    try {
        Parser parser(instance_file);
        data = parser.get_data();
        graph = std::make_shared<Graph>(data);
        hs = std::make_shared<HeuristicSolver>(data, graph);
        initial_solution = GenericPath();
    } catch(const std::exception& e) {
        std::cout << "An error occurred!" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void Program::prompt() {
    char* line;
    
    linenoiseHistoryLoad(".history");
    
    while((line = linenoise("tsppddl> ")) != NULL) {
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
        
        if(cmd_tokens[0] == "info" || cmd_tokens[0] == "i") {
            if(data != nullptr) {
                data->print();
            } else {
                std::cout << "No data file loaded!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "graph" || cmd_tokens[0] == "g") {
            if(graph != nullptr) {
                graph->print_data();
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "viz" || cmd_tokens[0] == "v") {
            if(graph != nullptr) {
                graph->print_graphviz("tsppddl.dot");
                system("dot -Tpng tsppddl.dot -o tsppddl.png && open tsppddl.png");
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "solve" || cmd_tokens[0] == "s") {
            if(graph != nullptr) {
                try {
                    MipSolver ms(graph, initial_solution);
                    ms.solve();
                } catch(const std::exception& e) {
                    std::cout << "An error occurred!" << std::endl;
                    std::cout << e.what() << std::endl;
                }
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
                
        if(cmd_tokens[0] == "labelling" || cmd_tokens[0] == "b") {
            if(graph != nullptr) {
                try {
                    LabellingSolver ls(graph);
                    ls.solve();
                } catch(const std::exception& e) {
                    std::cout << "An error occurred!" << std::endl;
                    std::cout << e.what() << std::endl;
                }
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "h2mindistance" || cmd_tokens[0] == "h2md") {
            if(graph != nullptr) {
                initial_solution = hs->solve_two_phases_min_distance();
                std::cout << initial_solution.cost << std::endl;
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "h2maxdistance" || cmd_tokens[0] == "h2Md") {
            if(graph != nullptr) {
                initial_solution = hs->solve_two_phases_max_distance();
                std::cout << initial_solution.cost << std::endl;
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "hRmaxLD" || cmd_tokens[0] == "hRMLD") {
            if(graph != nullptr) {
                initial_solution = hs->solve_max_regret_max_load_over_distance();
                std::cout << initial_solution.cost << std::endl;
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "hRminLD" || cmd_tokens[0] == "hRmLD") {
            if(graph != nullptr) {
                initial_solution = hs->solve_max_regret_min_load_times_distance();
                std::cout << initial_solution.cost << std::endl;
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
        }
        
        if(cmd_tokens[0] == "hall") {
            if(cmd_tokens.size() < 2) {
                std::cout << "Not enough parameters!" << std::endl;
            } else {
                GenericPath solution;
                for(int i : {10, 25, 50}) {
                    for(int j = 1; j <= 10; j++) {
                        std::stringstream file;
                        file << "data/" << cmd_tokens[1] << "_" << i << "_" << j << ".json";
                        load(file.str());
                        solution = hs->solve_two_phases_min_distance();
                        std::cout << initial_solution.cost << " ";
                        solution = hs->solve_two_phases_max_distance();
                        std::cout << initial_solution.cost << " ";
                        solution = hs->solve_max_regret_max_load_over_distance();
                        std::cout << initial_solution.cost << " ";
                        solution = hs->solve_max_regret_min_load_times_distance();
                        std::cout << initial_solution.cost << std::endl;
                    }
                }
            }
        }
        
        if(cmd_tokens[0] == "help" || cmd_tokens[0] == "?") {
            std::cout << "Commands:" << std::endl;
            std::cout << "\tquit [q]: quits the program" << std::endl;
            std::cout << "\tload [l] <string:data_file>: loads data from JSON" << std::endl;
            std::cout << "\tinfo [i]: displays information about the loaded data" << std::endl;
            std::cout << "\tgraph [g]: displays information about the generated graph" << std::endl;
            std::cout << "\tviz [v]: opens a graphviz representation of the graph" << std::endl;
            std::cout << "\tsolve [s]: launches the MIP solver" << std::endl;
            std::cout << "\tlabelling [b]: launches the reduced state space labelling algorithm" << std::endl;
            std::cout << "\th2mindistance [h2md]: launches the 2-phases min-distance heuristic" << std::endl;
            std::cout << "\th2maxdistance [h2Md]: launches the 2-phases min-distance heuristic" << std::endl;
            std::cout << "\thRminLD [hRmLD]: launches the max-regret min-(load*distance) heuristic" << std::endl;
            std::cout << "\thRmaxLD [hRMLD]: launches the max-regret max-(load/distance) heuristic" << std::endl;
            std::cout << "\thall [hall] <instances_prefix>: runs all the known heuristics over a set of instances" << std::endl;
            std::cout << "\thelp [?]: shows this help" << std::endl;
        }
        
        linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
    }
}

#endif