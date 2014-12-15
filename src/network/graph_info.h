#ifndef GRAPH_INFO_H
#define GRAPH_INFO_H

struct graph_info {
    int n;
    int capacity;
    
    graph_info() {}
    graph_info(int n, int capacity) : n{n}, capacity{capacity} {}
};

#endif