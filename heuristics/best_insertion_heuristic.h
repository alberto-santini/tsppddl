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
class best_insertion_heuristic : public heuristic {
    IC insertion_comparator;
    
public:
    best_insertion_heuristic(const tsp_graph& g, const IC& ic);
    path solve();
};

#include <heuristics/best_insertion_heuristic.tpp>

#endif