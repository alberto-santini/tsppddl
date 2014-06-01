#ifndef TWO_PHASES_HEURISTIC_H
#define TWO_PHASES_HEURISTIC_H

#include <memory>
#include <utility>
#include <vector>
#include <stdexcept>

#include <heuristics/generic_path.h>
#include <heuristics/generic_heuristic.h>
#include <heuristics/heuristic_helper.h>
#include <network/graph.h>
#include <parser/data.h>

template<typename RequestPricer, typename RequestComparator, typename InsertionPricer, typename InsertionComparator>
class TwoPhasesHeuristic : public GenericHeuristic<RequestPricer, RequestComparator, InsertionPricer, InsertionComparator> {
    bool perform_best_insertion_for_request(const int i);
        
public:
    TwoPhasesHeuristic(const std::shared_ptr<const RawData> rd, const RequestPricer request_pricer, const RequestComparator request_comparator, const InsertionPricer insertion_pricer, const InsertionComparator insertion_comparator) : GenericHeuristic<RequestPricer, RequestComparator, InsertionPricer, InsertionComparator>(rd, request_pricer, request_comparator, insertion_pricer, insertion_comparator) {}
    GenericPath solve();
};

#include <heuristics/two_phases_heuristic.tpp>

#endif