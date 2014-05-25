#ifndef PROGRAM_H
#define PROGRAM_H

#include <memory>
#include <string>

#include <network/graph.h>
#include <parser/parser.h>
#include <solver/heuristic_helper.h>

class Program {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<HeuristicHelper> hh;
    HeuristicSolution initial_solution;
    
public:
    Program();
    void prompt();
};

#endif