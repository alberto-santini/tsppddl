#ifndef VI_SEPARATOR_GENERALISED_ORDER_H
#define VI_SEPARATOR_GENERALISED_ORDER_H

#include <network/tsp_graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class vi_separator_generalised_order {
    const tsp_graph&        g;
    const program_params&   params;
    const ch::solution&     sol;
    IloEnv                  env;
    IloNumVarArray          x;
    
public:
    vi_separator_generalised_order( const tsp_graph& g,
                                    const program_params& params,
                                    const ch::solution& sol,
                                    const IloEnv& env,
                                    const IloNumVarArray& x) :
                                    g{g},
                                    params{params},
                                    sol{sol},
                                    env{env},
                                    x{x} {}
    std::vector<IloRange> separate_valid_cuts() const;
};

#endif