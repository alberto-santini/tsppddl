#ifndef VAILD_CUTS_CALLBACK_H
#define VALID_CUTS_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <utility>

class ValidCutsCallback : public IloCplex::UserCutCallbackI {
    IloNumVarArray x;
    std::shared_ptr<const Graph> g;
    double eps;
    
    CallbacksHelper::solution compute_x_values() const;
    
public:
    ValidCutsCallback(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const double eps) : IloCplex::UserCutCallbackI{env}, x{x}, g{g}, eps{eps} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback ValidCutsCallbackHandle(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const double eps) {
    return (IloCplex::Callback(new(env) ValidCutsCallback(env, x, g, eps)));
}

#endif