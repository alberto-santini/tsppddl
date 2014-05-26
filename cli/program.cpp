#ifndef PROGRAM_CPP
#define PROGRAM_CPP

#include <iostream>

#include <linenoise.h>
#include <boost/algorithm/string.hpp>

#include <cli/program.h>
#include <solver/mip_solver.h>
#include <solver/greedy_solver.h>
#include <solver/regret_heuristic_solver.h>
#include <solver/insertion_heuristic_solver.h>
#include <solver/min_distance_heuristic_solver.h>
#include <solver/labelling_solver.h>

Program::Program() {
    data = nullptr;
    graph = nullptr;
    hh = nullptr;
    initial_solution = std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1);
}

void Program::load(std::string instance_file) {
    try {
        Parser parser(instance_file);
        data = parser.get_data();
        graph = std::make_shared<Graph>(data);
        hh = std::make_shared<HeuristicHelper>(data, graph);
        initial_solution = std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1);
    } catch(const std::exception& e) {
        std::cout << "An error occurred!" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void Program::allheur(std::string instance_name) {
    std::string data_prefix = "data/";
    std::string data_suffix = ".json";
    
    for(int i : {10, 25, 50}) {
        for(int j = 1; j <= 10; j++) {
            std::stringstream file_name;
            file_name << data_prefix << instance_name << "_" << i << "_" << j << data_suffix;
            load(file_name.str());
            MinDistanceHeuristicSolver mdhs(data, graph, hh);
            HeuristicSolution s;
            s = mdhs.solve(false); std::cout << std::get<2>(s) << "\t";
            s = mdhs.solve(true); std::cout << std::get<2>(s) << "\t";
            RegretHeuristicSolver mrhs(data, graph, hh);
            s = mrhs.solve(); std::cout << std::get<2>(s) << "\t";
            InsertionHeuristicSolver bihs(data, graph, hh);
            s = bihs.solve();  std::cout << std::get<2>(s) << std::endl;
        }
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
                linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
            }
        }
        
        if(cmd_tokens[0] == "info" || cmd_tokens[0] == "i") {
            if(data != nullptr) {
                data->print();
            } else {
                std::cout << "No data file loaded!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "graph" || cmd_tokens[0] == "g") {
            if(graph != nullptr) {
                graph->print_data();
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "viz" || cmd_tokens[0] == "v") {
            if(graph != nullptr) {
                graph->print_graphviz("tsppddl.dot");
                system("dot -Tpng tsppddl.dot -o tsppddl.png && open tsppddl.png");
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
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
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "greedy" || cmd_tokens[0] == "gh") {
            if(graph != nullptr) {
                if(cmd_tokens.size() < 4) {
                    std::cout << "Not enough parameters!" << std::endl;
                } else {
                    GreedySolver gs(std::stoi(cmd_tokens[1]), std::stoi(cmd_tokens[2]), std::stod(cmd_tokens[3]), graph);
                    gs.solve();
                }
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "maxregret" || cmd_tokens[0] == "mrh") {
            if(graph != nullptr) {
                RegretHeuristicSolver mrhs(data, graph, hh);
                initial_solution = mrhs.solve();
                std::cout << std::get<2>(initial_solution);
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "bestinsertion" || cmd_tokens[0] == "bih") {
            if(graph != nullptr) {
                InsertionHeuristicSolver bihs(data, graph, hh);
                initial_solution = bihs.solve();
                std::cout << std::get<2>(initial_solution);
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "mmdistance" || cmd_tokens[0] == "mdh") {
            if(graph != nullptr) {
                if(cmd_tokens.size() < 2) {
                    std::cout << "Not enough parameters!" << std::endl;
                } else {
                    if(cmd_tokens[1] != "min" && cmd_tokens[1] != "max") {
                        std::cout << "Wrong parameter: " << cmd_tokens[1] << "!" << std::endl;
                    } else {
                        MinDistanceHeuristicSolver mdhs(data, graph, hh);
                        initial_solution = mdhs.solve(cmd_tokens[1] == "max");
                        std::cout << std::get<2>(initial_solution);
                    }
                }
            } else {
                std::cout << "No graph generated!" << std::endl;
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "allheur" || cmd_tokens[0] == "ah") {
            if(cmd_tokens.size() < 2) {
                std::cout << "Not enough parameters!" << std::endl;
            } else {
                allheur(cmd_tokens[1]);
            }
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
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
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
        
        if(cmd_tokens[0] == "help" || cmd_tokens[0] == "?") {
            std::cout << "Commands:" << std::endl;
            std::cout << "\tquit [q]: quits the program" << std::endl;
            std::cout << "\tload [l] <string:data_file>: loads data from JSON" << std::endl;
            std::cout << "\tinfo [i]: displays information about the loaded data" << std::endl;
            std::cout << "\tgraph [g]: displays information about the generated graph" << std::endl;
            std::cout << "\tviz [v]: opens a graphviz representation of the graph" << std::endl;
            std::cout << "\tsolve [s]: launches the MIP solver" << std::endl;
            std::cout << "\tgreedy [gh] <int:run_numbers> <int:best_arcs> <double:visitable_coeff>: launches the greedy heuristic solver" << std::endl;
            std::cout << "\tbestinsertion [bih]: launches the best insertion heuristic solver" << std::endl;
            std::cout << "\tmaxregret [mrh]: launches the max-regret heuristic solver" << std::endl;
            std::cout << "\tmmdistance [mdh] <min|max>: launches the min/max-distance heuristic solver" << std::endl;
            std::cout << "\tallheur [ah] <instance>: prints the results of all heuristics for an instance set" << std::endl;
            std::cout << "\tlabelling [b]: launches the reduced state space labelling algorithm" << std::endl; 
            std::cout << "\thelp [?]: shows this help" << std::endl;
            linenoiseHistoryAdd(line); linenoiseHistorySave(".history");
        }
    }
}

#endif