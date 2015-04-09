#ifndef SUBGRADIENT_SOLVER_H
#define SUBGRADIENT_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <parser/program_params.h>

#include <ilcplex/ilocplex.h>

#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace sg_compare {
    inline bool floating_equal(double x, double y) {
        double diff = std::abs(x - y); x = std::abs(x); y = std::abs(y); double largest = (y > x) ? y : x;
        return (diff <= largest * 10 * std::numeric_limits<double>::epsilon());
    }
    
    inline bool floating_optimal(double ub, double lb) {
        return (std::abs(ub - lb) < ub / pow(10,6));
    }
}

class subgradient_solver {
    const tsp_graph&        g;
    const program_params&   params;
    std::vector<path>       initial_solutions;
    double                  best_sol;
    
    void print_headers(std::ofstream& results_file) const;
    void print_result_row(std::ofstream& results_file, double result, double best_sol, int subgradient_iteration, double iteration_time, double cplex_obj, double obj_const_term, int violated_mtz, int loose_mtz, int tight_mtz, int violated_prec, int loose_prec, int tight_prec, double theta, double step_lambda, double step_mu, double avg_lambda_before, double avg_mu_before, double avg_l, double avg_m, bool improved, bool lg_mtz, bool lg_prec) const;
    void print_final_results(std::ofstream& results_file, double ub, double lb) const;
    void print_mult_dump_headers(std::ofstream& dump_file) const;
	void print_mult_dump(std::ofstream& dump_file, const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& lambda, const IloNumArray& x, const IloNumArray& t) const;
public:
    subgradient_solver(const tsp_graph& g, const program_params& params, const std::vector<path>& initial_solutions) : g{g}, params{params}, initial_solutions{initial_solutions}, best_sol{std::numeric_limits<double>::max()} {}
    void solve();
};

#endif