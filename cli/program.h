#ifndef PROGRAM_H
#define PROGRAM_H

#include <memory>
#include <string>

#include <network/graph.h>
#include <parser/parser.h>
#include <heuristics/heuristic_solver.h>

class Program {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<HeuristicSolver> hs;
    GenericPath initial_solution;
    
    void load(std::string instance_file);
    
public:
    Program();
    void prompt();
};

#endif