#ifndef HEURISTIC_H
#define HEURISTIC_H

#include <network/tsp_graph.h>
#include <network/path.h>

#include <vector>

class heuristic {
protected:
    const tsp_graph&    g;
    int                 n;
    std::vector<int>    remaining_requests;
    path                p;
    
public:
    heuristic(const tsp_graph& g);
};

#endif