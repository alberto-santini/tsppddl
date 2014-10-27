#ifndef HEURISTIC_HELPER_H
#define HEURISTIC_HELPER_H

#include <network/path.h>

#include <utility>

namespace HeuristicHelper {
    template<class IC>
    std::pair<bool, Path> insert(const Graph& g, const IC& insertion_comparator, int i, int x, int y, const Path& p, double bc, double bl);
}

#include <heuristics/heuristic_helper.tpp>

#endif