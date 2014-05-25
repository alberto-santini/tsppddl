#ifndef INSERTION_HEURISTIC_SOLVER_H
#define INSERTION_HEURISTIC_SOLVER_H

#include <vector>
#include <memory>

#include <parser/data.h>
#include <network/graph.h>
#include <solver/heuristic_helper.h>

class InsertionHeuristicSolver {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<HeuristicHelper> hh;
    
    std::pair<bool, placement> new_path_if_feasible(const int& x, const int& y, const int& i, const double& length, const int& load, const double& best_metric, const std::vector<int>& path, const std::vector<int>& partial_load) const;
    double test_path_cost(const std::vector<int>& path) const;

public:
    InsertionHeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph, const std::shared_ptr<HeuristicHelper> hh) : data(data), graph(graph), hh(hh) {}
    HeuristicSolution solve() const;
};

#endif