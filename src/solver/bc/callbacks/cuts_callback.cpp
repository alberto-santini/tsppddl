#include <solver/bc/callbacks/cuts_callback.h>
#include <solver/bc/callbacks/feasibility_cuts_separator.h>
#include <solver/bc/callbacks/vi_separator_capacity.h>
#include <solver/bc/callbacks/vi_separator_fork.h>
#include <solver/bc/callbacks/vi_separator_generalised_order.h>
#include <solver/bc/callbacks/vi_separator_subtour_elimination.h>

#include <chrono>
#include <ctime>
#include <ratio>

IloCplex::CallbackI* cuts_callback::duplicateCallback() const {
    return (new(getEnv()) cuts_callback(*this));
}

void cuts_callback::main() {
    using namespace std::chrono;
    
    auto node_number = getNnodes();
    auto sol_from_cplex = compute_x_values();
    
    if(sol_from_cplex.same_as_last_solution) {
        // Numerical error => Solution didn't change => Stop generating (always the same) cuts
        return;
    }
    
    getValues(last_solution, x);
    
    auto start_time = high_resolution_clock::now();
    auto feas_cuts = feasibility_cuts_separator::separate_feasibility_cuts(g, gr, sol_from_cplex.sol, x);
    auto end_time = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(end_time - start_time);
    data.time_spent_separating_feasibility_cuts += time_span.count();

    if(DEBUG && feas_cuts.size() > 0) {
        std::cerr << "cuts_callback.cpp::main() [" << node_number << "]\t Adding " << feas_cuts.size() << " feasibility cuts" << std::endl;
    }

    for(auto& cut : feas_cuts) {
        add(cut, IloCplex::UseCutForce).end();
        data.total_number_of_feasibility_cuts_added++;
    }
    
    bool separate_se = (
        (k_opt || params.bc.subtour_elim.enabled) &&
        should_separate(node_number, params.bc.subtour_elim.n1, params.bc.subtour_elim.n2, params.bc.subtour_elim.p1, params.bc.subtour_elim.p2, params.bc.subtour_elim.p3) &&
        (node_number < 2 || node_number != last_node_no_se)
    );
    if(separate_se) {
        auto sub_solv = vi_separator_subtour_elimination(g, params, sol_from_cplex.sol, env, x);
        auto start_time = high_resolution_clock::now();
        auto valid_cuts_1 = sub_solv.separate_valid_cuts();
        auto end_time = high_resolution_clock::now();
        auto time_span = duration_cast<duration<double>>(end_time - start_time);
        data.time_spent_separating_subtour_elimination_vi += time_span.count();

        if(DEBUG && valid_cuts_1.size() > 0) {
            std::cerr << "cuts_callback.cpp::main() [" << node_number << "]\t Adding " << valid_cuts_1.size() << " subtour elimination cuts" << std::endl;
        }

        if(valid_cuts_1.size() == 0) {
            last_node_no_se = node_number;
        }

        for(auto& cut : valid_cuts_1) {
            add(cut, IloCplex::UseCutForce).end();
            data.total_number_of_subtour_elimination_vi_added++;
        }
    }
    
    bool separate_go = (
        (k_opt || params.bc.generalised_order.enabled) &&
        should_separate(node_number, params.bc.generalised_order.n1, params.bc.generalised_order.n2, params.bc.generalised_order.p1, params.bc.generalised_order.p2, params.bc.generalised_order.p3) &&
        (node_number < 2 || node_number != last_node_no_go)
    );
    if(separate_go) {
        auto go_solv = vi_separator_generalised_order(g, sol_from_cplex.sol, env, x);
        auto start_time = high_resolution_clock::now();
        auto valid_cuts_2 = go_solv.separate_valid_cuts();
        auto end_time = high_resolution_clock::now();
        auto time_span = duration_cast<duration<double>>(end_time - start_time);
        data.time_spent_separating_generalised_order_vi += time_span.count();

        if(DEBUG && valid_cuts_2.size() > 0) {
            std::cerr << "cuts_callback.cpp::main() [" << node_number << "]\t Adding " << valid_cuts_2.size() << " generalised order cuts" << std::endl;
        }

        if(valid_cuts_2.size() == 0) {
            last_node_no_go = node_number;
        }

        for(auto& cut : valid_cuts_2) {
            add(cut, IloCplex::UseCutForce).end();
            data.total_number_of_generalised_order_vi_added++;
        }
    }
    
    bool separate_cap = (
        (k_opt || params.bc.capacity.enabled) &&
        should_separate(node_number, params.bc.capacity.n1, params.bc.capacity.n2, params.bc.capacity.p1, params.bc.capacity.p2, params.bc.capacity.p3) &&
        (node_number < 2 || node_number != last_node_no_cap)
    );
    if(separate_cap) {
        auto cap_solv = vi_separator_capacity(g, sol_from_cplex.sol, env, x);
        auto start_time = high_resolution_clock::now();
        auto valid_cuts_3 = cap_solv.separate_valid_cuts();
        auto end_time = high_resolution_clock::now();
        auto time_span = duration_cast<duration<double>>(end_time - start_time);
        data.time_spent_separating_capacity_vi += time_span.count();
    
        if(DEBUG && valid_cuts_3.size() > 0) {
            std::cerr << "cuts_callback.cpp::main() [" << node_number << "]\t Adding " << valid_cuts_3.size() << " capacity cuts" << std::endl;
        }
        
        if(valid_cuts_3.size() == 0) {
            last_node_no_cap = node_number;
        }
    
        for(auto& cut : valid_cuts_3) {
            add(cut, IloCplex::UseCutForce).end();
            data.total_number_of_capacity_vi_added++;
        }
    }
    
    bool separate_fork = (
        (k_opt || params.bc.fork.enabled) &&
        should_separate(node_number, params.bc.fork.n1, params.bc.fork.n2, params.bc.fork.p1, params.bc.fork.p2, params.bc.fork.p3) &&
        (node_number < 2 || node_number != last_node_no_fork)
    );
    if(separate_fork) {
        auto fork_solv = vi_separator_fork(g, sol_from_cplex.sol, env, x, params, data);
        auto start_time = high_resolution_clock::now();
        auto valid_cuts_5 = fork_solv.separate_valid_cuts();
        auto end_time = high_resolution_clock::now();
        auto time_span = duration_cast<duration<double>>(end_time - start_time);
        data.time_spent_separating_fork_vi += time_span.count();
        
        if(DEBUG && valid_cuts_5.size() > 0) {
            std::cerr << "cuts_callback.cpp::main() [" << node_number << "]\t Adding " << valid_cuts_5.size() << " fork cuts" << std::endl;
        }
        
        if(valid_cuts_5.size() == 0) {
            last_node_no_fork = node_number;
        }
        
        for(auto& cut : valid_cuts_5) {
            add(cut, IloCplex::UseCutForce).end();
            // Number of inequalities is increased by the separator
        }
    }
}

bool cuts_callback::should_separate(int node_number, int n1, int n2, double p1, double p2, double p3) const {
    std::uniform_real_distribution<double> d(0.0, 1.0);
    auto r = d(twister);
    
    assert(n1 <= n2);
    assert(p1 >= p2);
    assert(p2 >= p3);
    
    if(node_number <= n1) {
        auto x = static_cast<double>(node_number) / static_cast<double>(n1);
        auto t = p1 + x * (1.0 - p1);
        if(r <= t) { return true; }
    }
    
    if(n1 < node_number && node_number <= n2) {
        auto x = static_cast<double>(node_number - n1) / static_cast<double>(n2 - n1);
        auto t = p3 + x * (p2 - p3);
        if(r <= t) { return true; }
    }
    
    if(node_number > n2) {
        if(r <= p3) { return true; }
    }
    
    return false;
}

cuts_callback::solution_from_cplex cuts_callback::compute_x_values() const {
    auto n = g.g[graph_bundle].n;
    auto xvals = std::vector<std::vector<double>>(2*n+2, std::vector<double>(2*n+2, 0));
    auto is_integer = true;
    
    // Necessary condition to be the same is that they have the same length
    // This also catches the first run, where there is no "last solution"
    auto is_same_as_last = (last_solution.getSize() == x.getSize() ? true : false);

    auto col_index = 0;
    for(auto i = 0; i <= 2 * n + 1; i++) {
        for(auto j = 0; j <= 2 * n + 1; j++) {
            if(g.cost[i][j] >= 0) {
                auto n = getValue(x[col_index]);
                if(is_same_as_last && std::abs(n - last_solution[col_index]) > ch::eps(1)) {
                    is_same_as_last = false;
                }
                if(n > 0 + ch::eps(1)) {
                    if(n < 1 - ch::eps(1)) {
                        is_integer = false;
                    }
                    xvals[i][j] = n;
                } else {
                    xvals[i][j] = 0;
                }
                col_index++;
            }
         }
    }
    
    return solution_from_cplex(ch::solution(is_integer, xvals), is_same_as_last);
}