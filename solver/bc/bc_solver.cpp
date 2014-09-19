#include <global.h>
#include <solver/bc/callbacks/feasibility_cuts_callback.h>
#include <solver/bc/callbacks/feasibility_cuts_lazy_constraint.h>
#include <solver/bc/callbacks/valid_cuts_callback.h>
#include <solver/bc/bc_solver.h>

#include <ilcplex/ilocplex.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ratio>
#include <stdexcept>

BcSolver::BcSolver(const std::shared_ptr<const Graph>& g, const std::vector<Path>& initial_solutions, const std::string& instance_name) : g{g}, initial_solutions{initial_solutions}, instance_name{instance_name} {
    initial_solution = find_best_initial_solution();
    initial_solution.verify_feasible(g);
    int n {g->g[graph_bundle].n};
    
    initial_x_val = std::vector<std::vector<int>>(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    initial_y_val = std::vector<int>(2 * n + 2, 0);
    initial_t_val = std::vector<int>(2 * n + 2, 0);

    if(initial_solution.total_cost > 0) {
        for(int l = 0; l < 2 * n + 2; l++) {
            if(l < 2 * n + 2 - 1) { initial_x_val[initial_solution.path[l]][initial_solution.path[l+1]] = 1; }
            initial_y_val[initial_solution.path[l]] = initial_solution.load[l];
            initial_t_val[initial_solution.path[l]] = l;
        }
        initial_t_val[2 * n + 1] = 2 * n + 1;
    }
}

Path BcSolver::find_best_initial_solution() {
    return *std::min_element(initial_solutions.begin(), initial_solutions.end(), [] (const Path& p1, const Path& p2) -> bool { return (p1.total_cost < p2.total_cost); });
}

std::vector<std::vector<int>> BcSolver::solve_for_k_opt(const std::vector<std::vector<int>>& lhs, int rhs) {
    k_opt_lhs = lhs;
    k_opt_rhs = rhs;
    return solve(true, true);
}

void BcSolver::solve_with_branch_and_cut(bool tce) const {
    solve(false, tce);
}

std::vector<std::vector<int>> BcSolver::solve(bool k_opt, bool tce) const {
    using namespace std::chrono;

    long total_bb_nodes_explored;
    long number_of_cuts_added_at_root;
    double time_spent_at_root;
    double ub_at_root;
    double lb_at_root;
    double ub;
    double lb;
    double total_cplex_time;
    
    global::g_total_number_of_cuts_added = 0;
    global::g_total_time_spent_separating_cuts = 0;

    int n {g->g[graph_bundle].n};
    int Q {g->g[graph_bundle].capacity};
    demand_t d {g->demand};
    draught_t l {g->draught};
    cost_t c {g->cost};

    IloEnv env;
    IloModel model(env);

    IloNumVarArray variables_x(env);
    IloNumVarArray variables_y(env);
    IloNumVarArray variables_tt(env);
    
    IloRangeArray outdegree(env);
    IloRangeArray indegree(env);
    IloRangeArray y_upper(env);
    IloRangeArray y_lower(env);
    IloRangeArray load(env);
    IloRangeArray initial_load(env);
    IloRangeArray two_cycles_elimination(env);
    IloRangeArray k_opt_constraint(env);

    IloObjective obj = IloMinimize(env);

    #include <solver/bc/bc_setup_model.raw.cpp>
    
    model.add(obj);    
    model.add(variables_x);
    model.add(variables_y);
    
    model.add(outdegree);
    model.add(indegree);
    model.add(y_upper);
    model.add(y_lower);
    model.add(load);
    model.add(initial_load);
    if(tce) {
        model.add(two_cycles_elimination);
    }
    if(k_opt) {
        model.add(k_opt_constraint);
    }
    
    IloCplex cplex(model);
    
    // Add initial solution
    if(initial_solution.total_cost > 0) {
        IloNumVarArray initial_x_vars(env);
        IloNumArray initial_x_values(env);
        IloNumVarArray initial_y_vars(env);
        IloNumArray initial_y_values(env);
        
        int x_idx = 0;
        for(int i = 0; i <= 2 * n + 1; i++) {
            for(int j = 0; j <= 2 * n + 1; j++) {
                if(c[i][j] >= 0) {
                    initial_x_vars.add(variables_x[x_idx++]);
                    initial_x_values.add(initial_x_val[i][j]);
                }
            }
            initial_y_vars.add(variables_y[i]);
            initial_y_values.add(initial_y_val[i]);
        }
        
        cplex.addMIPStart(initial_x_vars, initial_x_values);
        cplex.addMIPStart(initial_y_vars, initial_y_values);
        
        initial_x_values.end(); initial_y_values.end();
        initial_x_vars.end(); initial_y_vars.end();
    }

    // Add callbacks to separate cuts
    std::shared_ptr<const Graph> gr_with_reverse {std::make_shared<const Graph>(g->make_reverse_graph())};
    if(global::g_search_for_cuts_every_n_nodes > 0) {
        cplex.use(FeasibilityCutsCallbackHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
        cplex.use(FeasibilityCutsLazyConstraintHandle(env, variables_x, g, gr_with_reverse, cplex.getParam(IloCplex::EpRHS)));
        // cplex.use(ValidCutsCallbackHandle(env, variables_x, g, cplex.getParam(IloCplex::EpRHS)));
    }

    // Export model to file
    if(!k_opt) { cplex.exportModel("model.lp"); } else { cplex.exportModel("kopt.lp"); }
    
    // Set CPLEX parameters
    cplex.setParam(IloCplex::TiLim, 3600);
    cplex.setParam(IloCplex::Threads, 1);
    cplex.setParam(IloCplex::NodeLim, 0);
    
    high_resolution_clock::time_point t_start {high_resolution_clock::now()};

    // Solve root node
    if(!cplex.solve()) {
        IloAlgorithm::Status status = cplex.getStatus();
        std::cerr << "BcSolver::solve()\tCPLEX problem encountered at root node." << std::endl;
        std::cerr << "BcSolver::solve()\tCPLEX status: " << status << std::endl;
        cplex.exportModel("model_err.lp");
        throw std::runtime_error("Some error occurred or the problem is infeasible.");
    } else {
        high_resolution_clock::time_point t_end {high_resolution_clock::now()};
        duration<double> time_span {duration_cast<duration<double>>(t_end - t_start)};
        
        time_spent_at_root = time_span.count();
        number_of_cuts_added_at_root = global::g_total_number_of_cuts_added;
        lb_at_root = cplex.getBestObjValue();
        ub_at_root = cplex.getObjValue();

        // Solve the rest of the BB tree
        cplex.setParam(IloCplex::NodeLim, 2100000000);
        if(!cplex.solve()) {
            IloAlgorithm::Status status = cplex.getStatus();
            std::cerr << "BcSolver::solve()\tCPLEX problem encountered after the root node." << std::endl;
            std::cerr << "BcSolver::solve()\tCPLEX status: " << status << std::endl;
            cplex.exportModel("model_err.lp");
            throw std::runtime_error("Some error occurred or the problem is infeasible.");
        }
    }

    high_resolution_clock::time_point t_end_total {high_resolution_clock::now()};
    duration<double> time_span_total {duration_cast<duration<double>>(t_end_total - t_start)};

    IloAlgorithm::Status status = cplex.getStatus();
    std::cerr << "BcSolver::solve()\tCPLEX status: " << status << std::endl;
    std::cerr << "BcSolver::solve()\tObjective value: " << cplex.getObjValue() << std::endl;

    total_bb_nodes_explored = cplex.getNnodes();
    lb = cplex.getBestObjValue();
    ub = cplex.getObjValue();
    total_cplex_time = time_span_total.count();
    
    if(k_opt) { global::g_total_time_spent_by_heuristics += total_cplex_time; }

    IloNumArray x(env);
    IloNumArray y(env);
    IloNumArray t(env);

    cplex.getValues(x, variables_x);
    cplex.getValues(y, variables_y);

    if(!k_opt) { std::cerr << "BcSolver::solve()\tSolution:" << std::endl; }

    // Get solution
    int col_index {0};
    std::vector<std::vector<int>> solution_x(2 * n + 2, std::vector<int>(2 * n + 2, 0));
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            if(c[i][j] >= 0) {
                if(x[col_index] > 0) {
                    solution_x[i][j] = 1;
                    if(!k_opt) { std::cerr << "\tx(" << i << ", " << j << ") = " << x[col_index] << std::endl; }
                }
                if(y[col_index] > 0) {
                    if(!k_opt) { std::cerr << "\ty(" << i << ", " << j << ") = " << y[col_index] << std::endl; }
                }
                col_index++;
            }
         }
    }

    // Print solution
    if(!k_opt) {
        std::ofstream results_file;
        results_file.open("results.txt", std::ios::out | std::ios::app);
        
        results_file << instance_name << "\t";
        results_file << global::g_search_for_cuts_every_n_nodes << "\t";
        results_file << std::boolalpha << tce << "\t";
        results_file << total_cplex_time << "\t";
        results_file << global::g_total_time_spent_by_heuristics << "\t";
        results_file << global::g_total_time_spent_separating_cuts << "\t";
        results_file << time_spent_at_root << "\t";
        results_file << initial_solution.total_cost << "\t";
        results_file << ub << "\t";
        results_file << lb << "\t";
        results_file << (ub - lb) / ub << "\t";
        results_file << ub_at_root << "\t";
        results_file << lb_at_root << "\t";
        results_file << (ub_at_root - lb_at_root) / ub_at_root << "\t";
        results_file << global::g_total_number_of_cuts_added << "\t";
        results_file << number_of_cuts_added_at_root << "\t";
        results_file << total_bb_nodes_explored << std::endl;
        
        results_file.close();
    }

    env.end();
    
    return solution_x;
}