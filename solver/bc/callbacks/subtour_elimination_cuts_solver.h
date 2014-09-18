#ifndef SUBTOUR_ELIMINATION_CUTS_SOLVER_H
#define SUBTOUR_ELIMINATION_CUTS_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <vector>

namespace SubtourEliminationCutsSolver {
    std::vector<IloRange> separate_valid_cuts(std::shared_ptr<const Graph> g, CallbacksHelper::solution sol, IloNumVarArray x, double eps);
}

#endif