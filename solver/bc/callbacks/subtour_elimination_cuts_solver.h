#ifndef SUBTOUR_ELIMINATION_CUTS_SOLVER_H
#define SUBTOUR_ELIMINATION_CUTS_SOLVER_H

#include <network/graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class SubtourEliminationCutsSolver {
    const Graph& g;
    const ProgramParams& params;
    ch::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    
    ch::sets_info pi;
    ch::sets_info sigma;
    
    int tot_number_of_iterations;
    int tabu_duration;
    int n;
    
    void update_info(ch::sets_info& set, const ch::sets_info& best, int bn, int iter);
    
    void add_pi_cut_if_violated(std::vector<IloRange>& cuts, const ch::sets_info& pi);
    void add_sigma_cut_if_violated(std::vector<IloRange>& cuts, const ch::sets_info& sigma);
    
    void add_groetschel_pi_cut_if_violated(std::vector<IloRange>& cuts, const ch::sets_info& pi);
    void add_groetschel_sigma_cut_if_violated(std::vector<IloRange>& cuts, const ch::sets_info& sigma);
    
    void add_or_remove_from_pi_sets(ch::sets_info& pi, int i);
    void add_or_remove_from_sigma_sets(ch::sets_info& sigma, int i);
    
    void recalculate_pi_sums(ch::sets_info& pi);
    void recalculate_sigma_sums(ch::sets_info& sigma);
    
    double calculate_groetschel_lhs_pi(const std::vector<int>& my_S, const ch::sets_info& pi);
    double calculate_groetschel_lhs_sigma(const std::vector<int>& my_S, const ch::sets_info& pi);
    
    void actually_add_groetschel_pi_cut(std::vector<IloRange>& cuts, const std::vector<int>& my_S, const ch::sets_info& pi);
    void actually_add_groetschel_sigma_cut(std::vector<IloRange>& cuts, const std::vector<int>& my_S, const ch::sets_info& sigma);
    
public:
    SubtourEliminationCutsSolver(const Graph& g, const ProgramParams& params, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps);
    std::vector<IloRange> separate_valid_cuts();
};

#endif