#ifndef CUTS_LAZY_CONSTRAINT_H
#define CUTS_LAZY_CONSTRAINT_H

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

class CutsLazyConstraint : public IloCplex::LazyConstraintCallbackI {
    IloEnv env;
    IloNumVarArray x;
    std::shared_ptr<const Graph> g;
    std::shared_ptr<const Graph> gr;
    double eps;
    bool apply_valid_cuts;
    
    ch::solution compute_x_values() const;

public:
    CutsLazyConstraint(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, double eps, bool apply_valid_cuts) : IloCplex::LazyConstraintCallbackI{env}, env{env}, x{x}, g{g}, gr{gr}, eps{eps}, apply_valid_cuts{apply_valid_cuts} {}

    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback CutsLazyConstraintHandle(const IloEnv& env, const IloNumVarArray& x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, double eps, bool apply_valid_cuts) {
    return (IloCplex::Callback(new(env) CutsLazyConstraint(env, x, g, gr, eps, apply_valid_cuts)));
}

#endif