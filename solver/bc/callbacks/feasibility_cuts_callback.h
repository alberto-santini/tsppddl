#ifndef FEASIBILITY_CUTS_CALLBACK_H
#define FEASIBILITY_CUTS_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <utility>

class FeasibilityCutsCallback : public IloCplex::UserCutCallbackI {
    IloNumVarArray x;
    std::shared_ptr<const Graph> g;
    std::shared_ptr<const Graph> gr;
    double eps;
    
    CallbacksHelper::solution compute_x_values() const;
    
public:
    FeasibilityCutsCallback(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) : IloCplex::UserCutCallbackI{env}, x{x}, g{g}, gr{gr}, eps{eps} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback FeasibilityCutsCallbackHandle(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) {
    return (IloCplex::Callback(new(env) FeasibilityCutsCallback(env, x, g, gr, eps)));
}

#endif