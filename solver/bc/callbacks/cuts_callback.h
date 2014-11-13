#ifndef CUTS_CALLBACK_H
#define CUTS_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <utility>

class CutsCallback : public IloCplex::UserCutCallbackI {
    IloEnv env;
    IloNumVarArray x;
    const Graph& g;
    const Graph& gr;
    double eps;
    const ProgramParams& params;
    
    ch::solution compute_x_values() const;
    
public:
    CutsCallback(const IloEnv& env, const IloNumVarArray& x, const Graph& g, const Graph& gr, double eps, const ProgramParams& params) : IloCplex::UserCutCallbackI{env}, env{env}, x{x}, g{g}, gr{gr}, eps{eps}, params{params} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback CutsCallbackHandle(const IloEnv& env, const IloNumVarArray& x, const Graph& g, const Graph& gr, double eps, const ProgramParams& params) {
    return (IloCplex::Callback(new(env) CutsCallback(env, x, g, gr, eps, params)));
}

#endif