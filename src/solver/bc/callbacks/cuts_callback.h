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
    struct solution_from_cplex {
        ch::solution    sol;
        bool            same_as_last_solution;
        
        solution_from_cplex(ch::solution sol, bool same_as_last_solution) : sol{std::move(sol)}, same_as_last_solution{same_as_last_solution} {}
    };
    
    IloEnv                  env;
    IloNumVarArray          x;
    bool                    k_opt;
    tsp_graph&              g;
    const tsp_graph&        gr;
    const program_params&   params;
    program_data&           data;
    IloNumArray&            last_solution;
    
    solution_from_cplex compute_x_values() const;
    
public:
    cuts_callback(const IloEnv& env, const IloNumVarArray& x, bool k_opt, tsp_graph& g, const tsp_graph& gr, const program_params& params, program_data& data, IloNumArray& last_solution) : IloCplex::UserCutCallbackI{env}, env{env}, x{x}, k_opt{k_opt}, g{g}, gr{gr}, params{params}, data{data}, last_solution{last_solution} {}
    
    IloCplex::CallbackI* duplicateCallback() const;
    void main();
};

inline IloCplex::Callback cuts_callback_handle(const IloEnv& env, const IloNumVarArray& x, bool k_opt, tsp_graph& g, const tsp_graph& gr, const program_params& params, program_data& data, IloNumArray& last_solution) {
    return (IloCplex::Callback(new(env) cuts_callback(env, x, k_opt, g, gr, params, data, last_solution)));
}

#endif