#ifndef VI_SEPARATOR_CAPACITY_H
#define VI_SEPARATOR_CAPACITY_H

#include <network/tsp_graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <boost/optional.hpp>

#include <vector>

class vi_separator_capacity {
    const tsp_graph&        g;
    const program_params&   params;
    const ch::solution&     sol;
    IloEnv                  env;
    IloNumVarArray          x;
    
    std::vector<int>        S;
    std::vector<int>        T;
    
    struct best_node {
        int     node_n;
        double  flow;
        
        best_node(int node_n, double flow) : node_n{node_n}, flow{flow} {}
    };
    
    boost::optional<best_node> best_pickup_node_for_S() const;
    boost::optional<best_node> best_delivery_node_for_S() const;
    boost::optional<best_node> best_pickup_node_for_T() const;
    boost::optional<best_node> best_delivery_node_for_T() const;
    double calculate_lhs() const;
    double calculate_rhs() const;
    IloRange add_cut(double rhs_val) const;
    
public:
    vi_separator_capacity(const tsp_graph& g, const program_params& params, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x);
    std::vector<IloRange> separate_valid_cuts();
};

#endif