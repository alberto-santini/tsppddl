#ifndef HEURISTIC_WITH_ORDERED_REQUESTS_H
#define HEURISTIC_WITH_ORDERED_REQUESTS_H

#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <boost/optional.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

struct default_requests_vector_sorter {
    bool is_custom = false;
    void sort_in_place(std::vector<int>& whatever) const {}
};

template<class RequestComparator, class InsertionComparator, class RequestsVectorSorter = default_requests_vector_sorter>
class heuristic_with_ordered_requests : public heuristic {
    InsertionComparator insertion_comparator;
    RequestsVectorSorter requests_vector_sorter;
    bool insert(int req);

public:
    heuristic_with_ordered_requests(const tsp_graph& g, const RequestComparator& rc, const InsertionComparator& ic, const RequestsVectorSorter& requests_vector_sorter = RequestsVectorSorter());
    boost::optional<path> solve();
};

#include <heuristics/heuristic_with_ordered_requests.tpp>

#endif