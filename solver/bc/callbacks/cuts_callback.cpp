#include <global.h>
#include <solver/bc/callbacks/cuts_callback.h>
#include <solver/bc/callbacks/capacity_solver.h>
#include <solver/bc/callbacks/feasibility_cuts_max_flow_solver.h>
#include <solver/bc/callbacks/generalized_order_solver.h>
#include <solver/bc/callbacks/subtour_elimination_cuts_solver.h>

#include <fstream>
#include <iostream>

IloCplex::CallbackI* CutsCallback::duplicateCallback() const {
    return (new(getEnv()) CutsCallback(*this));
}

void CutsCallback::main() {
    auto node_number = getNnodes();
    auto sol = compute_x_values();
    
    if(sol.is_integer || (node_number % params.bc.cut_every_n_nodes == 0)) {
        auto feas_cuts = FeasibilityCutsMaxFlowSolver::separate_feasibility_cuts(g, gr, sol, x, eps);

        for(auto& cut : feas_cuts) {
            add(cut, IloCplex::UseCutForce).end();
            global::g_total_number_of_feasibility_cuts_added++;
        }
        
        if(params.bc.separate_subtour_elimination) {
            auto sub_solv = SubtourEliminationCutsSolver(g, params, sol, env, x, eps);
            auto valid_cuts_1 = sub_solv.separate_valid_cuts();

            for(auto& cut : valid_cuts_1) {
                add(cut, IloCplex::UseCutForce).end();
                global::g_total_number_of_subtour_cuts_added++;
            }
        }
        
        if(params.bc.separate_precedence) {
            auto go_solv = GeneralizedOrderSolver(g, sol, env, x, eps);
            auto valid_cuts_2 = go_solv.separate_valid_cuts();

            for(auto& cut : valid_cuts_2) {
                add(cut, IloCplex::UseCutForce).end();
                global::g_total_number_of_generalized_order_cuts_added++;
            }
        }
        
        if(params.bc.separate_capacity) {
            auto cap_solv = CapacitySolver(g, sol, env, x, eps);
            auto valid_cuts_3 = cap_solv.separate_valid_cuts();
        
            for(auto& cut : valid_cuts_3) {
                add(cut, IloCplex::UseCutForce).end();
                global::g_total_number_of_capacity_cuts_added++;
            }
        }
    }
}

ch::solution CutsCallback::compute_x_values() const {
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