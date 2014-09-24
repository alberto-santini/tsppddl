#include <global.h>
#include <solver/bc/callbacks/cuts_lazy_constraint.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

IloCplex::CallbackI* CutsLazyConstraint::duplicateCallback() const {
    return (new(getEnv()) CutsLazyConstraint(*this));
}

void CutsLazyConstraint::main() {
    auto sol = compute_x_values();
        
    auto feas_cuts = FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(g, gr, sol, x, eps);
    
    // std::cout << "Adding " << feas_cuts.size() << " feasibility lazy constraints" << std::endl;
    
    for(IloRange cut : feas_cuts) {
        add(cut, IloCplex::UseCutForce).end();
        global::g_total_number_of_cuts_added++;
    }
    
    if(apply_valid_cuts) {
        SubtourEliminationCutsSolver sub_solv {g, sol, env, x, eps};
        auto valid_cuts = sub_solv.separate_valid_cuts();

        // std::cout << "Adding " << valid_cuts.size() << " valid lazy constraints" << std::endl;

        for(IloRange& cut : valid_cuts) {
            add(cut, IloCplex::UseCutForce).end();
            global::g_total_number_of_cuts_added++;
        }
    }
}

ch::solution CutsLazyConstraint::compute_x_values() const {
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
    
    return ch::solution(is_integer, xvals);
}