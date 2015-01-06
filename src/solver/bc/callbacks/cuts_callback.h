#ifndef CUTS_CALLBACK_H
#define CUTS_CALLBACK_H

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

class cuts_callback : public IloCplex::UserCutCallbackI {
    IloEnv                  env;
    IloNumVarArray          x;
    tsp_graph&              g;
    const tsp_graph&        gr;
    const program_params&   params;
    program_data&           data;
    
    ch::solution compute_x_values() const;
    
public:
    cuts_callback(const IloEnv& env, const IloNumVarArray& x, tsp_graph& g, const tsp_graph& gr, const program_params& params, program_data& data) : IloCplex::UserCutCallbackI{env}, env{env}, x{x}, g{g}, gr{gr}, params{params}, data{data} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback cuts_callback_handle(const IloEnv& env, const IloNumVarArray& x, tsp_graph& g, const tsp_graph& gr, const program_params& params, program_data& data) {
    return (IloCplex::Callback(new(env) cuts_callback(env, x, g, gr, params, data)));
}

#endif