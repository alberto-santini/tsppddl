#ifndef GENERIC_HEURISTIC_H
#define GENERIC_HEURISTIC_H

#include <memory>
#include <utility>
#include <vector>
#include <stdexcept>

#include <heuristics/generic_path.h>
#include <heuristics/heuristic_helper.h>
#include <network/graph.h>
#include <parser/data.h>

template<typename RequestPricer, typename RequestComparator, typename InsertionPricer, typename InsertionComparator>
class GenericHeuristic {
protected:
    std::shared_ptr<const RawData> rd;
    
    std::vector<int> unevaded_requests;
    GenericPath p;
    
    RequestPricer request_pricer;
    RequestComparator request_comparator;
    InsertionPricer insertion_pricer;
    InsertionComparator insertion_comparator;
            
public:
    GenericHeuristic(const std::shared_ptr<const RawData> rd, const RequestPricer request_pricer, const RequestComparator request_comparator, const InsertionPricer insertion_pricer, const InsertionComparator insertion_comparator);
};

#include <heuristics/generic_heuristic.tpp>

#endif