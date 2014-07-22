#ifndef PATH_H
#define PATH_H

#include <network/graph.h>

#include <vector>

class Path {
public:
    std::vector<int> path;
    std::vector<int> load;
    int total_load;
    int total_cost;
    
    Path() : Path(0) {}
    Path(unsigned int expected_length);
    
    void verify_feasible(std::shared_ptr<const Graph> g);
};

#endif