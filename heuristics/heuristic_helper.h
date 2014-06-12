#ifndef HEURISTIC_HELPER_H
#define HEURISTIC_HELPER_H

#include <network/path.h>

#include <memory>
#include <utility>

namespace HeuristicHelper {
    template<class IC>
    std::pair<bool, Path> insert(const std::shared_ptr<const Graph> g, const IC insertion_comparator, const int i, const int x, const int y, const Path p, const double bc, const double bl);
}

#include <heuristics/heuristic_helper.tpp>

#endif