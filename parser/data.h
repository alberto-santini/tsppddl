#ifndef DATA_H
#define DATA_H

#include <memory>
#include <vector>

#include <network/port.h>
#include <network/request.h>

class Data {
public:
    int num_ports;
    int num_requests;
    int capacity;
    std::vector<std::shared_ptr<Port>> ports;
    std::vector<std::shared_ptr<Request>> requests;
    std::vector<std::vector<double>> distances;
    std::string data_file_name;
    
    Data(const int num_ports, const int num_requests, const int capacity, const std::vector<std::shared_ptr<Port>> ports, const std::vector<std::shared_ptr<Request>> requests, const std::vector<std::vector<double>> distances, std::string data_file_name) : num_ports(num_ports), num_requests(num_requests), capacity(capacity), ports(ports), requests(requests), distances(distances), data_file_name(data_file_name) {}
    std::shared_ptr<Port> port_by_id(const int id) const;
    void check_port_ids_consistent_with_depot() const;
    void print() const;
};

#endif