#ifndef PARSER_H
#define PARSER_H

#include <network/graph.h>

#include <string>

class Parser {
    std::string file_name;
    
public:
    Parser(const std::string& file_name) : file_name{file_name} {}
    Graph generate_graph() const;
};

#endif