#ifndef GRAPH_INFO_H
#define GRAPH_INFO_H

struct GraphInfo {
    int n;
    int capacity;
    
    GraphInfo() {}
    GraphInfo(const int n, const int capacity) : n{n}, capacity{capacity} {}
};

#endif