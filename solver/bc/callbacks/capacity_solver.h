#ifndef CAPACITY_SOLVER_H
#define CAPACITY_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <boost/optional.hpp>

#include <vector>

class CapacitySolver {
    const Graph& g;
    ch::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    
    std::vector<int> S;
    std::vector<int> T;
    
    boost::optional<ch::best_node> best_pickup_node_for_S() const;
    boost::optional<ch::best_node> best_delivery_node_for_S() const;
    boost::optional<ch::best_node> best_pickup_node_for_T() const;
    boost::optional<ch::best_node> best_delivery_node_for_T() const;
    double calculate_lhs() const;
    double calculate_rhs() const;
    IloRange add_cut(double rhs_val) const;
    
public:
    CapacitySolver(const Graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps);
    std::vector<IloRange> separate_valid_cuts();
};

#endif