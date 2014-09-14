#include <global.h>
#include <solver/bc/callbacks/feasibility_cuts_lazy_constraint.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>

IloCplex::CallbackI* FeasibilityCutsLazyConstraint::duplicateCallback() const {
    return (new(getEnv()) FeasibilityCutsLazyConstraint(*this));
}

void FeasibilityCutsLazyConstraint::main() {
    long node_number = getNnodes();
    auto sol = compute_x_values();
    
    // std::ofstream cuts_file;
    // cuts_file.open("cuts.txt", std::ios::out | std::ios::app);
    // cuts_file << "LZY " << std::setw(6) << node_number << std::endl;
    // cuts_file.close();
    
    auto cuts = FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(g, gr, sol, x, eps);
    
    for(IloRange cut : cuts) {
        add(cut, IloCplex::UseCutForce).end();
        global::g_total_number_of_cuts_added++;
    }
}

CallbacksHelper::solution FeasibilityCutsLazyConstraint::compute_x_values() const {
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