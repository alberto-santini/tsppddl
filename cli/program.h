#ifndef PROGRAM_H
#define PROGRAM_H

#include <memory>
#include <string>

#include <network/graph.h>
#include <parser/parser.h>

class Program {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::tuple<std::vector<int>, std::vector<int>, int> initial_solution;
    
public:
    Program();
    void prompt();
};

#endif