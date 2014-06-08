#ifndef MAX_REGRET_HEURISTIC_H
#define MAX_REGRET_HEURISTIC_H

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <stdexcept>

#include <heuristics/generic_path.h>
#include <heuristics/generic_heuristic.h>
#include <heuristics/heuristic_helper.h>
#include <network/graph.h>
#include <parser/data.h>

template<typename InsertionPricer, typename InsertionComparator, typename RegretCalculator>
class MaxRegretHeuristic : public GenericHeuristic<DefaultRequestPricer, DefaultRequestComparator, InsertionPricer, InsertionComparator> {
    RegretCalculator regret_calculator;
    
public:
    MaxRegretHeuristic(std::shared_ptr<const RawData> rd, const InsertionPricer insertion_pricer, const InsertionComparator insertion_comparator, const RegretCalculator regret_calculator) : GenericHeuristic<DefaultRequestPricer, DefaultRequestComparator, InsertionPricer, InsertionComparator>(rd, DefaultRequestPricer(), DefaultRequestComparator(), insertion_pricer, insertion_comparator), regret_calculator(regret_calculator) {}
    GenericPath solve();
};

#include <heuristics/max_regret_heuristic.tpp>

#endif