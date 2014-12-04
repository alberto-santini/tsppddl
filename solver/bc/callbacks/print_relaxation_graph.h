#ifndef PRINT_RELAXATION_GRAPH_H
#define PRINT_RELAXATION_GRAPH_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

class PrintRelaxationGraphCallback : public IloCplex::HeuristicCallbackI {
    IloEnv env;
    const IloNumVarArray& x;
    const IloNumVarArray& y;
    std::string instance_name;
    const Graph& g;
            
public:
    PrintRelaxationGraphCallback(const IloEnv& env, const IloNumVarArray& x, const IloNumVarArray& y, std::string instance_name, const Graph& g) : IloCplex::HeuristicCallbackI{env}, env{env}, x{x}, y{y}, instance_name{instance_name}, g{g} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback PrintRelaxationGraphCallbackHandle(const IloEnv& env, const IloNumVarArray& x, const IloNumVarArray& y, std::string instance_name, const Graph& g) {
    return (IloCplex::Callback(new(env) PrintRelaxationGraphCallback(env, x, y, std::move(instance_name), g)));
}

#endif