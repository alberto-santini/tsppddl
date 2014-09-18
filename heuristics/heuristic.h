#ifndef HEURISTIC_H
#define HEURISTIC_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <vector>

class Heuristic {
protected:
    std::shared_ptr<const Graph> g;
    int n;
    std::vector<int> remaining_requests;
    Path p;
    
public:
    Heuristic(const std::shared_ptr<const Graph>& g);
    void print_requests() const;
};

#endif