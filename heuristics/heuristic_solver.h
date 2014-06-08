#ifndef HEURISTIC_SOLVER_H
#define HEURISTIC_SOLVER_H

#include <memory>
#include <vector>

#include <heuristics/generic_path.h>
#include <heuristics/raw_data.h>
#include <heuristics/two_phases_heuristic.h>
#include <heuristics/best_insertion_heuristic.h>

class HeuristicSolver {
    std::shared_ptr<Data> data;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<RawData> rd;
    
public:
    HeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph);
    
    GenericPath solve_two_phases_min_distance() const;
    GenericPath solve_two_phases_max_distance() const;
    GenericPath solve_max_regret_max_load_over_distance() const;
    GenericPath solve_max_regret_min_load_times_distance() const;
    GenericPath solve_best_insertion_load_over_distance() const;
    GenericPath solve_best_insertion_load_times_distance() const;
};

#endif