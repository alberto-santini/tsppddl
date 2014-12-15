#ifndef CUTS_CALLBACK_H
#define CUTS_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/tsp_graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <utility>

class cuts_callback : public IloCplex::UserCutCallbackI {
    IloEnv env;
    IloNumVarArray x;
    tsp_graph& g;
    const tsp_graph& gr;
    double eps;
    const program_params& params;
    
    ch::solution compute_x_values() const;
    
public:
    cuts_callback(const IloEnv& env, const IloNumVarArray& x, tsp_graph& g, const tsp_graph& gr, double eps, const program_params& params) : IloCplex::UserCutCallbackI{env}, env{env}, x{x}, g{g}, gr{gr}, eps{eps}, params{params} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback cuts_callback_handle(const IloEnv& env, const IloNumVarArray& x, tsp_graph& g, const tsp_graph& gr, double eps, const program_params& params) {
    return (IloCplex::Callback(new(env) cuts_callback(env, x, g, gr, eps, params)));
}

#endif