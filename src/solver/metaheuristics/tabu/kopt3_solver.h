#ifndef KOPT3_SOLVER_H
#define KOPT3_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <solver/metaheuristics/tabu/tabu_solver.h>

#include <utility>
#include <vector>

class kopt3_solver {
    tsp_graph& g;
    
    boost::optional<path> exec_3opt(const path& p, const std::vector<int>& i, const std::vector<int>& j);
    
public:
    kopt3_solver(tsp_graph& g) : g{g} {}
    tabu_solver::tabu_and_non_tabu_solutions solve(const path& starting_solution, const std::vector<tabu_solver::tabu_move>& tabu_moves);
};

#endif