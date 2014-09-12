#ifndef FEASIBILITY_CUTS_LAZY_CONSTRAINT_H
#define FEASIBILITY_CUTS_LAZY_CONSTRAINT_H

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

class FeasibilityCutsLazyConstraint : public IloCplex::LazyConstraintCallbackI {
    IloNumVarArray x;
    std::shared_ptr<const Graph> g;
    std::shared_ptr<const Graph> gr;
    double eps;
    
    CallbacksHelper::solution compute_x_values() const;

public:
    FeasibilityCutsLazyConstraint(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) : IloCplex::LazyConstraintCallbackI{env}, x{x}, g{g}, gr{gr}, eps{eps} {}

    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback FeasibilityCutsLazyConstraintHandle(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) {
    return (IloCplex::Callback(new(env) FeasibilityCutsLazyConstraint(env, x, g, gr, eps)));
}

#endif