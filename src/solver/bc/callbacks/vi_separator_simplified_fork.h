#ifndef VI_SEPARATOR_SIMPLIFIED_FORK_H
#define VI_SEPARATOR_SIMPLIFIED_FORK_H

#include <network/tsp_graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class vi_separator_simplified_fork {
    const tsp_graph&    g;
    const ch::solution& sol;
    IloEnv              env;
    IloNumVarArray      x;
    double              eps;
    
    struct CutDefiningSets {
        std::vector<int> S;
        std::vector<int> T;
    
        CutDefiningSets(std::vector<int> S, std::vector<int> T) : S{S}, T{T} {}
    };
    
    CutDefiningSets scan_for_infork(int i, int j) const;
    CutDefiningSets scan_for_outfork(int i, int j) const;
    
public:
    vi_separator_simplified_fork(const tsp_graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps) : g{g}, sol{sol}, env{env}, x{x}, eps{eps} {}
    std::vector<IloRange> separate_valid_cuts() const;
};

#endif