#ifndef BEST_INSERTION_HEURISTIC_H
#define BEST_INSERTION_HEURISTIC_H

#include <global.h>
#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <chrono>
#include <ctime>
#include <limits>
#include <ratio>

template<class IC>
class BestInsertionHeuristic : public Heuristic {
    IC insertion_comparator;
    
public:
    BestInsertionHeuristic(const Graph& g, const IC& ic);
    Path solve();
};

#include <heuristics/best_insertion_heuristic.tpp>

#endif