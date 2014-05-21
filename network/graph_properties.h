#ifndef GRAPH_PROPERTIES_H
#define GRAPH_PROPERTIES_H

struct GraphProperties {
    int num_requests;
    int ship_capacity;
    
    GraphProperties(const int num_requests = -1, const int ship_capacity = -1) : num_requests(num_requests), ship_capacity(ship_capacity) {}
};

#endif