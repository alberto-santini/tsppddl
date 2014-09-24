#ifndef FEASIBILITY_CUTS_MAX_FLOW_SOLVER_H
#define FEASIBILITY_CUTS_MAX_FLOW_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <vector>

namespace FeasibilityCutsMaxFlowSolver {
    std::vector<IloRange> separate_feasibility_cuts(std::shared_ptr<const Graph> g, std::shared_ptr<const Graph> gr, ch::solution sol, IloNumVarArray x, double eps);
}

#endif