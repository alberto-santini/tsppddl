#ifndef PROGRAM_H
#define PROGRAM_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <memory>
#include <string>
#include <vector>

class program {
    tsp_graph g;
    program_params  params;
    
    void load(const std::string& params_filename, const std::string& instance_filename);
    void print_usage();

public:
    program(const std::vector<std::string>& args);
};

#endif