#ifndef CUTS_LAZY_CONSTRAINT_H
#define CUTS_LAZY_CONSTRAINT_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/tsp_graph.h>
#include <parser/program_params.h>
#include <program/program_data.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <utility>

class cuts_lazy_constraint : public IloCplex::LazyConstraintCallbackI {
    IloEnv              env;
    IloNumVarArray      x;
    const tsp_graph&    g;
    const tsp_graph&    gr;
    double              eps;
    program_data&       data;
    
    ch::solution compute_x_values() const;

public:
    cuts_lazy_constraint(const IloEnv& env, const IloNumVarArray& x, const tsp_graph& g, const tsp_graph& gr, double eps, program_data& data) : IloCplex::LazyConstraintCallbackI{env}, env{env}, x{x}, g{g}, gr{gr}, eps{eps}, data{data} {}

    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback cuts_lazy_constraint_handle(const IloEnv& env, const IloNumVarArray& x, const tsp_graph& g, const tsp_graph& gr, double eps, program_data& data) {
    return (IloCplex::Callback(new(env) cuts_lazy_constraint(env, x, g, gr, eps, data)));
}

#endif