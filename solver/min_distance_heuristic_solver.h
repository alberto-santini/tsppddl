#ifndef MIN_DISTANCE_HEURISTIC_SOLVER_H
#define MIN_DISTANCE_HEURISTIC_SOLVER_H

#include <memory>
#include <vector>

#include <parser/data.h>
#include <network/graph.h>
#include <solver/heuristic_helper.h>

class MinDistanceHeuristicSolver {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<HeuristicHelper> hh;

public:
    MinDistanceHeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph, const std::shared_ptr<HeuristicHelper> hh) : data(data), graph(graph), hh(hh) {}
    HeuristicSolution solve(bool inverse_order = false) const;
};

#endif