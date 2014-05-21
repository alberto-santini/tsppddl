#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include <vector>

#include <parser/data.h>

class Parser {    
    std::string data_file_name;
    std::shared_ptr<Port> port_by_id(const std::vector<std::shared_ptr<Port>>& ports, const int id) const;
    
public:    
    Parser(const std::string data_file_name) : data_file_name(data_file_name) {}
    std::shared_ptr<Data> get_data() const;
};

#endif