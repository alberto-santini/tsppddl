#ifndef SUBGRADIENT_SOLVER_H
#define SUBGRADIENT_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <memory>
#include <string>
#include <vector>

class SubgradientSolver {
    std::shared_ptr<const Graph> g;
    std::string instance_name;
    const std::vector<Path>& initial_solutions;
    double best_sol;
    int iteration_limit;
    
public:
    SubgradientSolver(std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, std::string instance_name, int iteration_limit) : g{g}, initial_solutions{initial_solutions}, instance_name{instance_name}, iteration_limit{iteration_limit} {}
    void solve();
};

#endif