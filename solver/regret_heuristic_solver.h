#ifndef REGRET_HEURISTIC_SOLVER_H
#define REGRET_HEURISTIC_SOLVER_H

#include <vector>
#include <memory>

#include <parser/data.h>
#include <network/graph.h>

class RegretHeuristicSolver {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    
    std::vector<std::vector<double>> distance;
    std::vector<int> draught;
    int ship_capacity;
    int n;
    
    std::tuple<bool, std::vector<int>, std::vector<int>, double, int, double> new_path_if_feasible(const int& x, const int& y, const int& i, const double& length, const int& load, const double& second_best_metric, const std::vector<int>& path, const std::vector<int>& partial_load) const;
    double test_path_cost(const std::vector<int>& path) const;

public:
    std::vector<int> demand;
    
    RegretHeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph);
    std::tuple<std::vector<int>, std::vector<int>, int> solve() const;
};

#endif