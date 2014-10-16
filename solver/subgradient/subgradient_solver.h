#ifndef SUBGRADIENT_SOLVER_H
#define SUBGRADIENT_SOLVER_H

#include <network/graph.h>
#include <network/path.h>

#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace sg_compare {
    inline bool floating_equal(double x, double y) {
        double diff = std::abs(x - y); x = std::abs(x); y = std::abs(y); double largest = (y > x) ? y : x;
        if(diff <= largest * 10 * std::numeric_limits<double>::epsilon()) { return true; } else { return false; }
    }
    
    inline bool floating_optimal(double ub, double lb) {
        return (std::abs(ub - lb) < ub / pow(10,6));
    }
}

class SubgradientSolver {
    std::shared_ptr<const Graph> g;
    std::vector<Path> initial_solutions;
    std::string instance_name;
    int iteration_limit;
    double best_sol;
    
    void print_headers(std::ofstream& results_file) const;
    void print_result_row(std::ofstream& results_file, double result, double best_sol, int subgradient_iteration, double iteration_time, double cplex_obj, double obj_const_term, int violated_mtz, int loose_mtz, int tight_mtz, int violated_prec, int loose_prec, int tight_prec, double theta, double step_lambda, double step_mu, double avg_lambda_before, double avg_mu_before, double avg_l, double avg_m, bool improved, bool lg_mtz, bool lg_prec) const;
    void print_final_results(std::ofstream& results_file, double ub, double lb) const;
public:
    SubgradientSolver(std::shared_ptr<const Graph> g, const std::vector<Path>& initial_solutions, std::string instance_path, int iteration_limit);
    void solve(bool lg_mtz, bool lg_prec);
};

#endif