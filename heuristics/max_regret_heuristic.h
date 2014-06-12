#ifndef MAX_REGRET_HEURISTIC_H
#define MAX_REGRET_HEURISTIC_H

#include <heuristics/heuristic.h>
#include <heuristics/heuristic_helper.h>

#include <limits>
#include <memory>
#include <stdexcept>

template<class IC, class RG>
class MaxRegretHeuristic : public Heuristic {
    IC insertion_comparator;
    RG regret_calculator;
    
public:
    MaxRegretHeuristic(const std::shared_ptr<const Graph> g, const IC ic, const RG rg) : Heuristic{g}, insertion_comparator{ic}, regret_calculator{rg} {}
    Path solve();
};

#include <heuristics/max_regret_heuristic.tpp>

#endif