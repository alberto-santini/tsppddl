#ifndef NODE_H
#define NODE_H

#include <memory>

#include <network/port.h>

class Node {
public:
    int id;
    std::shared_ptr<Port> port;
    int demand;
    
    Node(const int id, std::shared_ptr<Port> port, const int demand) : id(id), port(port), demand(demand) {}
};

#endif