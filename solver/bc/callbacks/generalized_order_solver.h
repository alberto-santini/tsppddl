#ifndef GENRALIZED_ORDER_SOLVER_H
#define GENRALIZED_ORDER_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class GeneralizedOrderSolver {
    const Graph& g;
    ch::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    
public:
    GeneralizedOrderSolver(const Graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps) : g{g}, sol{sol}, env{env}, x{x}, eps{eps} {}
    std::vector<IloRange> separate_valid_cuts() const;
};

#endif