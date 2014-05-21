#ifndef REQUEST_H
#define REQUEST_H

#include <memory>

#include <network/port.h>

class Request {
public:
    std::shared_ptr<Port> origin;
    std::shared_ptr<Port> destination;
    int demand;
    
    Request(const std::shared_ptr<Port> origin, const std::shared_ptr<Port> destination, const int demand) : origin(origin), destination(destination), demand(demand) {}
};

#endif