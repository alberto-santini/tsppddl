#ifndef PRINT_RELAXATION_GRAPH_CALLBACK_H
#define PRINT_RELAXATION_GRAPH_CALLBACK_H

/*************************************************
 INCLUDE THIS OR CPLEX WILL PANIC... DRAMA QUEEN!
*************************************************/
#include <cstring>
/************************************************/

#include <network/tsp_graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

class print_relaxation_graph_callback : public IloCplex::HeuristicCallbackI {
    IloEnv env;
    const IloNumVarArray& x;
    const IloNumVarArray& y;
    std::string instance_name;
    const tsp_graph& g;
            
public:
    print_relaxation_graph_callback(const IloEnv& env, const IloNumVarArray& x, const IloNumVarArray& y, std::string instance_name, const tsp_graph& g) : IloCplex::HeuristicCallbackI{env}, env{env}, x{x}, y{y}, instance_name{instance_name}, g{g} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback print_relaxation_graph_callback_handle(const IloEnv& env, const IloNumVarArray& x, const IloNumVarArray& y, std::string instance_name, const tsp_graph& g) {
    return (IloCplex::Callback(new(env) print_relaxation_graph_callback(env, x, y, std::move(instance_name), g)));
}

#endif