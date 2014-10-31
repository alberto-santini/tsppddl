#ifndef PARSER_H
#define PARSER_H

#include <network/graph.h>
#include <parser/program_params.h>

#include <string>
#include <utility>

class Parser {
    std::string params_file_name;
    std::string instance_file_name;
    
public:
    Parser(std::string params_file_name, std::string instance_file_name) : params_file_name{std::move(params_file_name)}, instance_file_name{std::move(instance_file_name)} {}
    
    Graph generate_graph() const;
    ProgramParams read_program_params() const;
};

#endif