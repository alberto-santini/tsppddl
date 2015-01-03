#ifndef HEURISTIC_WITH_ORDERED_REQUESTS_H
#define HEURISTIC_WITH_ORDERED_REQUESTS_H

#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <boost/optional.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

template<class RC, class IC>
class heuristic_with_ordered_requests : public heuristic {
    IC insertion_comparator;
    bool insert(int req);

public:
    heuristic_with_ordered_requests(const tsp_graph& g, const RC& rc, const IC& ic);
    boost::optional<path> solve();
};

#include <heuristics/heuristic_with_ordered_requests.tpp>

#endif