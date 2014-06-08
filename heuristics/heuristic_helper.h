#ifndef HEURISTICS_HELPER_H
#define HEURISTICS_HELPER_H

#include <memory>
#include <utility>
#include <vector>

#include <heuristics/generic_path.h>
#include <heuristics/raw_data.h>

struct DefaultRequestPricer {
    int operator()(const int r) {
        return r;
    }
};

struct DefaultRequestComparator {
    bool operator()(const int r1, const int r2) {
        return (r1 > r2);
    }
};

namespace HeuristicHelper {
    template<typename InsertionPricer, typename InsertionComparator>
    std::pair<bool, GenericPath> feasible_and_improves(const int i, const int x, const int y, const int best_cost, const int best_load, const GenericPath& p, const std::shared_ptr<const RawData> rd, InsertionPricer insertion_pricer, InsertionComparator insertion_comparator);
    
    int test_path_cost(const GenericPath& p, const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph);
}

#include <heuristics/heuristic_helper.tpp>

#endif