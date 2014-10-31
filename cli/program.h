#ifndef PROGRAM_H
#define PROGRAM_H

#include <network/graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <memory>
#include <string>
#include <vector>

class Program {
    std::unique_ptr<const Graph> g;
    std::unique_ptr<const ProgramParams> params;
    std::vector<Path> heuristic_solutions;
    
    void load(const std::string& params_filename, const std::string& instance_filename);
    void print_usage();

public:
    Program() : g{nullptr} {}
    void autorun(const std::vector<std::string>& args);
};

#endif