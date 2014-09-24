#ifndef SUBTOUR_ELIMINATION_CUTS_SOLVER_H
#define SUBTOUR_ELIMINATION_CUTS_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <vector>

class SubtourEliminationCutsSolver {
    std::shared_ptr<const Graph> g;
    ch::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    ch::sets_info pi;
    ch::sets_info sigma;
    int tot_number_of_iterations;
    int tabu_duration;
    int n;
    
    void update_info(ch::sets_info& set, ch::sets_info best, int bn, int iter);
    void add_pi_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info pi);
    void add_sigma_cut_if_violated(std::vector<IloRange>& cuts, ch::sets_info sigma);
    void add_or_remove_from_pi_sets(ch::sets_info& pi, int i);
    void add_or_remove_from_sigma_sets(ch::sets_info& sigma, int i);
    void recalculate_pi_sums(ch::sets_info& pi, std::vector<std::vector<double>> xvals);
    void recalculate_sigma_sums(ch::sets_info& pi, std::vector<std::vector<double>> xvals);
    
public:
    SubtourEliminationCutsSolver(std::shared_ptr<const Graph> g, ch::solution sol, IloEnv env, IloNumVarArray x, double eps);
    std::vector<IloRange> separate_valid_cuts();
};

#endif