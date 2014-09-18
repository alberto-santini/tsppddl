#ifndef PROGRAM_H
#define PROGRAM_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <string>
#include <vector>

class Program {
    std::shared_ptr<const Graph> g;
    std::vector<Path> heuristic_solutions;
    
    void load(const std::string& filename);

public:
    Program() : g{nullptr} {}
    void autorun(const std::vector<std::string>& args);
};

#endif