#ifndef MAX_REGRET_HEURISTIC_H
#define MAX_REGRET_HEURISTIC_H

#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <limits>
#include <stdexcept>

template<class IC, class RG>
class max_regret_heuristic : public heuristic {
    IC insertion_comparator;
    RG regret_calculator;
    
public:
    max_regret_heuristic(const tsp_graph& g, const IC& ic, const RG& rg) : heuristic{g}, insertion_comparator(ic), regret_calculator(rg) {}
    path solve();
};

#include <heuristics/max_regret_heuristic.tpp>

#endif