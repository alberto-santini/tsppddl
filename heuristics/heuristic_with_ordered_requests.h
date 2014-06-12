#ifndef HEURISTIC_WITH_ORDERED_REQUESTS_H
#define HEURISTIC_WITH_ORDERED_REQUESTS_H

#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

template<class RC, class IC>
class HeuristicWithOrderedRequests : public Heuristic {
    IC insertion_comparator;
    bool insert(const int req);

public:
    HeuristicWithOrderedRequests(const std::shared_ptr<const Graph> g, const RC rc, const IC ic);
    Path solve();
};

#include <heuristics/heuristic_with_ordered_requests.tpp>

#endif