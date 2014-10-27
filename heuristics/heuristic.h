#ifndef HEURISTIC_H
#define HEURISTIC_H

#include <network/graph.h>
#include <network/path.h>

#include <vector>

class Heuristic {
protected:
    const Graph& g;
    int n;
    std::vector<int> remaining_requests;
    Path p;
    
public:
    Heuristic(const Graph& g);
    void print_requests() const;
};

#endif