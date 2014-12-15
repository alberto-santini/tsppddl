#ifndef HEURISTIC_HELPER_H
#define HEURISTIC_HELPER_H

#include <network/path.h>

#include <utility>

namespace heuristic_helper {
    template<class IC>
    std::pair<bool, path> insert(const tsp_graph& g, const IC& insertion_comparator, int i, int x, int y, const path& p, double bc, double bl);
}

#include <heuristics/heuristic_helper.tpp>

#endif