#include <global.h>
#include <solver/bc/callbacks/cuts_lazy_constraint.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>
#include <solver/bc/callbacks/generalized_order_solver.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

IloCplex::CallbackI* CutsLazyConstraint::duplicateCallback() const {
    return (new(getEnv()) CutsLazyConstraint(*this));
}

void CutsLazyConstraint::main() {
    auto sol = compute_x_values();
        
    auto feas_cuts = FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(g, gr, sol, x, eps);
        
    for(IloRange cut : feas_cuts) {
        add(cut, IloCplex::UseCutForce).end();
        global::g_total_number_of_feasibility_cuts_added++;
    }
    
    if(apply_valid_cuts) {
        SubtourEliminationCutsSolver sub_solv {g, params, sol, env, x, eps};
        auto valid_cuts_1 = sub_solv.separate_valid_cuts();

        for(auto& cut : valid_cuts_1) {
            add(cut, IloCplex::UseCutForce).end();
            global::g_total_number_of_subtour_cuts_added++;
        }
        
        auto go_solv = GeneralizedOrderSolver(g, sol, env, x, eps);
        auto valid_cuts_2 = go_solv.separate_valid_cuts();
        
        for(auto& cut : valid_cuts_2) {
            add(cut, IloCplex::UseCutForce).end();
            global::g_total_number_of_generalized_order_cuts_added++;
        }
    }
}

ch::solution CutsLazyConstraint::compute_x_values() const {
    auto n = g.g[graph_bundle].n;
    auto xvals = std::vector<std::vector<double>>(2*n+2, std::vector<double>(2*n+2, 0));
    auto is_integer = true;

    auto col_index = 0;
    for(auto i = 0; i <= 2 * n + 1; i++) {
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(g.cost[i][j] >= 0) {
                auto n = getValue(x[col_index++]);
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