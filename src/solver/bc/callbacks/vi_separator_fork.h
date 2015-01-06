#ifndef FORK_SOLVER_H
#define FORK_SOLVER_H

#include <network/tsp_graph.h>
#include <program/program_data.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <utility>
#include <vector>

class vi_separator_fork {
    tsp_graph&          g;
    const ch::solution& sol;
    IloEnv              env;
    IloNumVarArray      x;
    
    struct lhs_info {
        double                              value;
        std::vector<std::pair<int, int>>    arcs;
    
        lhs_info(double value, std::vector<std::pair<int, int>> arcs) : value{value}, arcs{std::move(arcs)} {}
    };
    
    boost::optional<IloRange> generate_cut(const std::vector<int>& path, const std::vector<int>& S, const std::vector<int>& T) const;
    lhs_info calculate_lhs(const std::vector<int>& path, const std::vector<int>& S, const std::vector<int>& T) const;
    void extend_path(const std::vector<int>& path, std::vector<std::vector<int>>& paths_to_check) const;
    std::vector<int> create_set_T_for(const std::vector<int>& path);
    std::vector<int> create_set_S_for(std::vector<int>& path, const std::vector<int>& T);
    bool is_infeasible(const std::vector<int>& path);
    
public:
    vi_separator_fork(tsp_graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x) : g{g}, sol{sol}, env{env}, x{x} {}
    std::vector<IloRange> separate_valid_cuts();
};

#endif