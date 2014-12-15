#ifndef PARSER_H
#define PARSER_H

#include <network/tsp_graph.h>
#include <parser/program_params.h>

#include <string>
#include <utility>

class parser {
    std::string params_file_name;
    std::string instance_file_name;
    
public:
    parser(std::string params_file_name, std::string instance_file_name) : params_file_name{std::move(params_file_name)}, instance_file_name{std::move(instance_file_name)} {}
    
    tsp_graph generate_tsp_graph() const;
    program_params  read_program_params() const;
};

#endif