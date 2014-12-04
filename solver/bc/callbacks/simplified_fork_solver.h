#ifndef SIMPLIFIED_FORK_SOLVER_H
#define SIMPLIFIED_FORK_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

struct CutDefiningSets {
    std::vector<int> S;
    std::vector<int> T;
    
    CutDefiningSets(std::vector<int> S, std::vector<int> T) : S{S}, T{T} {}
};

class SimplifiedForkSolver {
    const Graph& g;
    ch::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    
    CutDefiningSets scan_for_infork(int i, int j) const;
    CutDefiningSets scan_for_outfork(int i, int j) const;
    
public:
    SimplifiedForkSolver(const Graph& g, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps) : g{g}, sol{sol}, env{env}, x{x}, eps{eps} {}
    std::vector<IloRange> separate_valid_cuts() const;
};

#endif