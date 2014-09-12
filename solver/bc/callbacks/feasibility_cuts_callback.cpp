#include <global.h>
#include <solver/bc/callbacks/feasibility_cuts_callback.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>

IloCplex::CallbackI* FeasibilityCutsCallback::duplicateCallback() const {
    return (new(getEnv()) FeasibilityCutsCallback(*this));
}

void FeasibilityCutsCallback::main() {
    long node_number = getNnodes();
    auto sol = compute_x_values();
    
    if(sol.is_integer || (node_number % global::g_search_for_cuts_every_n_nodes == 0)) {
        auto cuts = FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(g, gr, sol, x, eps);
        
        for(IloRange& cut : cuts) {
            add(cut, IloCplex::UseCutForce).end();
            global::g_total_number_of_cuts_added++;
        }
    }
}

CallbacksHelper::solution FeasibilityCutsCallback::compute_x_values() const {
    int n {g->g[graph_bundle].n};
    cost_t c {g->cost};
    std::vector<std::vector<double>> xvals(2*n+2, std::vector<double>(2*n+2, 0));
    bool is_integer {true};

    int col_index {0};
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                IloNum n = getValue(x[col_index++]);
                if(n > eps) {
                    if(n < 1 - eps) {
                        is_integer = false;
                    }
                    xvals[i][j] = n;
                } else {
                    xvals[i][j] = 0;
                }
            }
         }
    }
    
    return CallbacksHelper::solution(is_integer, xvals);
}