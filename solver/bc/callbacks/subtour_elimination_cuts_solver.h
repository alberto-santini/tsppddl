#ifndef SUBTOUR_ELIMINATION_CUTS_SOLVER_H
#define SUBTOUR_ELIMINATION_CUTS_SOLVER_H

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <boost/container/vector.hpp>
#include <boost/range/algorithm/find.hpp>

#include <memory>
#include <vector>

typedef boost::container::vector<bool> bvec;
typedef std::vector<int> ivec;

struct sets_info {
    bvec in_S;
    bvec in_tabu;
    ivec tabu_start;
    bvec in_fs; double fs;  // First sum
    bvec in_ss; double ss;  // Second sum
    bvec in_ts; double ts;  // Third sum
    double lhs;
    
    sets_info(bvec in_S, bvec in_tabu, ivec tabu_start, bvec in_fs, double fs, bvec in_ss, double ss, bvec in_ts, double ts, double lhs) : in_S{in_S}, in_tabu{in_tabu}, tabu_start{tabu_start}, in_fs{in_fs}, fs{fs}, in_ss{in_ss}, ss{ss}, in_ts{in_ts}, ts{ts}, lhs{lhs} {
        in_tabu[0] = true; // 0 is a dummy value anyways
    }
    
    bool empty_S() const {
        return (boost::find(in_S, true) != boost::end(in_S));
    }
    
    int first_non_tabu() const {
        return (boost::find(in_tabu, false) - boost::begin(in_tabu));
    }
};

class SubtourEliminationCutsSolver {
    std::shared_ptr<const Graph> g;
    CallbacksHelper::solution sol;
    IloEnv env;
    IloNumVarArray x;
    double eps;
    sets_info pi;
    sets_info sigma;
    int tot_number_of_iterations;
    int tabu_duration;
    int n;
    
    void update_info(sets_info& set, sets_info best, int bn, int iter);
    void add_pi_cut_if_violated(std::vector<IloRange>& cuts, sets_info pi);
    void add_sigma_cut_if_violated(std::vector<IloRange>& cuts, sets_info sigma);
    void add_or_remove_from_pi_sets(sets_info& pi, int i);
    void add_or_remove_from_sigma_sets(sets_info& sigma, int i);
    void recalculate_pi_sums(sets_info& pi, std::vector<std::vector<double>> xvals);
    void recalculate_sigma_sums(sets_info& pi, std::vector<std::vector<double>> xvals);
    
public:
    SubtourEliminationCutsSolver(std::shared_ptr<const Graph> g, CallbacksHelper::solution sol, IloEnv env, IloNumVarArray x, double eps);
    std::vector<IloRange> separate_valid_cuts();
};

#endif