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
    
    Path() : path{std::vector<int>()}, load{std::vector<int>()}, total_load{0}, total_cost{0} {}
    Path(const Graph& g, const std::vector<std::vector<int>>& x);
        
    void verify_feasible(const Graph& g) const;
};

#endif