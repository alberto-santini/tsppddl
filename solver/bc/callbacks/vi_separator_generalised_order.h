#ifndef VI_SEPARATOR_GENERALISED_ORDER_H
#define VI_SEPARATOR_GENERALISED_ORDER_H

#include <network/tsp_graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class vi_separator_generalised_order {
    const tsp_graph& g;
    const ch::solution& sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    
public:
    vi_separator_generalised_order(const tsp_graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps) : g{g}, sol{sol}, env{env}, x{x}, eps{eps} {}
    std::vector<IloRange> separate_valid_cuts() const;
};

#endif