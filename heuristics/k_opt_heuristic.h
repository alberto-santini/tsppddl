#ifndef K_OPT_HEURISTIC_H
#define K_OPT_HEURISTIC_H

#include <network/graph.h>
#include <network/path.h>

#include <vector>

class KOptHeuristic {
    const Graph& g;
    int k;
    std::vector<Path> initial_solutions;
    
    std::vector<std::vector<int>> get_x_values(const Path& p) const;
    Path get_path(const std::vector<std::vector<int>>& x) const;
    
public:
    KOptHeuristic(const Graph& g, int k, const std::vector<Path>& initial_solutions) : g{g}, k{k}, initial_solutions{initial_solutions} {}
    Path solve() const;
    Path solve_with_multiple_columns() const;
};

#endif