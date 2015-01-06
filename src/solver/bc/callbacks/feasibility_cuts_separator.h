#ifndef FEASIBILITY_CUTS_SEPARATOR_H
#define FEASIBILITY_CUTS_SEPARATOR_H

#include <network/tsp_graph.h>
#include <program/program_data.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

namespace feasibility_cuts_separator {
    std::vector<IloRange> separate_feasibility_cuts(const tsp_graph& g, const tsp_graph& gr, const ch::solution& sol, const IloNumVarArray& x);
}

#endif