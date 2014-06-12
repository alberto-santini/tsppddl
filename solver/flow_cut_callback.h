#ifndef FLOW_CUT_CALLBACK_H
#define FLOW_CUT_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/graph.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>

class FlowCutCallback : public IloCplex::UserCutCallbackI {
    IloNumVarArray x;
    std::shared_ptr<const Graph> g;
    std::shared_ptr<const Graph> gr;
    double eps;
    
    std::vector<std::vector<double>> compute_x_values() const;
    
public:
    FlowCutCallback(IloEnv env, const IloNumVarArray x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) : IloCplex::UserCutCallbackI{env}, x{x}, g{g}, gr{gr}, eps{eps} {}
    
    // Virtual functions of IloCplex::UserCutCallbackI
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback FlowCutCallbackHandle(IloEnv env, const IloNumVarArray x, const std::shared_ptr<const Graph> g, const std::shared_ptr<const Graph> gr, const double eps) {
    return (IloCplex::Callback(new(env) FlowCutCallback(env, x, g, gr, eps)));
}

#endif