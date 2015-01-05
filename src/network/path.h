#ifndef PATH_H
#define PATH_H

#include <network/tsp_graph.h>

#include <vector>

struct path {
    std::vector<int> path_v;
    std::vector<int> load_v;
    int total_load;
    int total_cost;
    
    static constexpr double eps = 0.0001;
    
    path() : path_v{std::vector<int>()}, load_v{std::vector<int>()}, total_load{0}, total_cost{0} {}
    path(const tsp_graph& g, const std::vector<std::vector<int>>& x);
        
    void verify_feasible(const tsp_graph& g) const;
};

#endif