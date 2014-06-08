#ifndef BEST_INSERTION_HEURISTIC_H
#define BEST_INSERTION_HEURISTIC_H

#include <memory>
#include <utility>
#include <vector>
#include <stdexcept>

#include <heuristics/generic_path.h>
#include <heuristics/generic_heuristic.h>
#include <heuristics/heuristic_helper.h>
#include <network/graph.h>
#include <parser/data.h>

template<typename InsertionPricer, typename InsertionComparator>
class BestInsertionHeuristic : public GenericHeuristic<DefaultRequestPricer, DefaultRequestComparator, InsertionPricer, InsertionComparator> {
        
public:
    BestInsertionHeuristic(const std::shared_ptr<const RawData> rd, const InsertionPricer insertion_pricer, const InsertionComparator insertion_comparator) : GenericHeuristic<DefaultRequestPricer, DefaultRequestComparator, InsertionPricer, InsertionComparator>(rd, DefaultRequestPricer(), DefaultRequestComparator(), insertion_pricer, insertion_comparator) {}
    GenericPath solve();
};

#include <heuristics/best_insertion_heuristic.tpp>

#endif