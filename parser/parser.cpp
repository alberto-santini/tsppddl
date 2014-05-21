#ifndef PARSER_CPP
#define PARSER_CPP

#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>

#include <parser/parser.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

std::shared_ptr<Data> Parser::get_data() const {
    using namespace boost::property_tree;
    
    ptree pt;
    read_json(data_file_name, pt);
    
    int num_ports = pt.get<int>("num_ports");
    
    std::vector<std::shared_ptr<Port>> ports;
    BOOST_FOREACH(const ptree::value_type& port, pt.get_child("ports")) {
        ports.push_back(std::make_shared<Port>(
            port.second.get<int>("id"),
            port.second.get<int>("draught"),
            port.second.get<bool>("depot"),
            port.second.get<int>("x"),
            port.second.get<int>("y")
        ));
    }
    
    int num_requests = pt.get<int>("num_requests");
    
    std::vector<std::shared_ptr<Request>> requests;
    BOOST_FOREACH(const ptree::value_type& request, pt.get_child("requests")) {
        requests.push_back(std::make_shared<Request>(
            port_by_id(ports, request.second.get<int>("origin")),
            port_by_id(ports, request.second.get<int>("destination")),
            request.second.get<int>("demand")
        ));
    }
    
    int capacity = pt.get<int>("capacity");
    
    std::vector<std::vector<double>> distances;
    BOOST_FOREACH(const ptree::value_type& distances_row, pt.get_child("distances")) {
        std::vector<double> distances_r;
        BOOST_FOREACH(const ptree::value_type& distance, distances_row.second.get_child("")) {
            distances_r.push_back(distance.second.get<double>(""));
        }
        distances.push_back(distances_r);
    }
    
    return std::make_shared<Data>(num_ports, num_requests, capacity, ports, requests, distances, data_file_name);
}

std::shared_ptr<Port> Parser::port_by_id(const std::vector<std::shared_ptr<Port>>& ports, const int id) const {
    auto port_iterator = std::find_if(ports.begin(), ports.end(),
        [&id] (const std::shared_ptr<Port>& p) { return (p->id == id); }
    );
    
    if(port_iterator == ports.end()) {
        throw std::runtime_error("Can't find port with id: " + std::to_string(id) + ".");
    } else {
        return *port_iterator;
    }
}

#endif