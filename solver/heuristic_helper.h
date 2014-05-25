#ifndef HEURISTIC_HELPER_H
#define HEURISTIC_HELPER_H

#include <network/graph.h>
#include <parser/data.h>

// Path, Loads, Cost
typedef std::tuple<std::vector<int>, std::vector<int>, double> HeuristicSolution;

// Path, Loads, Length, Load, Metric
typedef std::tuple<std::vector<int>, std::vector<int>, double, int, double> placement;

class HeuristicHelper {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    
public:
    std::vector<std::vector<double>> distance;
    std::vector<int> draught;
    std::vector<int> demand;
    int ship_capacity;
    int n;
    
    HeuristicHelper(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph);
    
    std::pair<bool, placement> new_path_if_feasible(const int& x, const int& y, const int& i, const double& length, const int& load, const double& best_metric, const std::vector<int>& path, const std::vector<int>& partial_load) const;
    double test_path_cost(const std::vector<int>& path) const;
};

#endif