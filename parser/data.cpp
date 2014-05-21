#ifndef DATA_CPP
#define DATA_CPP

#include <iostream>
#include <algorithm>

#include <parser/data.h>

void Data::print() const {
    std::cout << "Number of ports: " << num_ports << std::endl;
    std::cout << "Ports:" << std::endl;
    for(const std::shared_ptr<Port>& p : ports) {
        std::cout << "\t" << p->id << ": (" << p->x << ", " << p->y << "), draught: " << p->draught << std::endl;
    }
    std::cout << "Number of requests: " << num_requests << std::endl;
    std::cout << "Requests:" << std::endl;
    for(const std::shared_ptr<Request>& req : requests) {
        std::cout << "\t" << req->origin->id << " -> " << req->destination->id << ", demand: " << req->demand << std::endl;
    }
    std::cout << "Ship capacity: " << capacity << std::endl;
    std::cout << "Distances:" << std::endl;
    for(int i = 0; i < num_ports; i++) {
        for(int j = i + 1; j < num_ports; j++) {
            std::cout << "\tFrom " << i << " to " << j << ": " << distances[i][j] << std::endl;
        }
    }
}

void Data::check_port_ids_consistent_with_depot() const {
    for(const std::shared_ptr<Port>& port : ports) {
        if(port->id == 0 && !port->is_depot) {
            throw std::runtime_error("Port 0 is required to be the depot.");
        }
        if(port->is_depot && port->id != 0) {
            throw std::runtime_error("Port " + std::to_string(port->id) + " is market as depot.");
        }
    }
}

std::shared_ptr<Port> Data::port_by_id(const int id) const {
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