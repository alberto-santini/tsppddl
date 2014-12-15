#ifndef VI_SEPARATOR_SUBTOUR_ELIMINATION_H
#define VI_SEPARATOR_SUBTOUR_ELIMINATION_H

#include <network/tsp_graph.h>
#include <parser/program_params.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <vector>

class vi_separator_subtour_elimination {
    const tsp_graph&        g;
    const program_params&   params;
    const ch::solution&     sol;
    IloEnv                  env;
    IloNumVarArray          x;
    double                  eps;
    
    using bvec = boost::container::vector<bool>;
    using ivec = std::vector<int>;
    
    struct sets_info {
        int     n;
        bvec    in_S;
        bvec    in_tabu;
        ivec    tabu_start;
        bvec    in_fs; double fs;  // First sum
        bvec    in_ss; double ss;  // Second sum
        bvec    in_ts; double ts;  // Third sum
        double  lhs;
        
        sets_info() {}
        
        sets_info(int n, const bvec& in_S, const bvec& in_tabu, const ivec& tabu_start, const bvec& in_fs, double fs, const bvec& in_ss, double ss, const bvec& in_ts, double ts, double lhs) : n{n}, in_S{in_S}, in_tabu{in_tabu}, tabu_start{tabu_start}, in_fs{in_fs}, fs{fs}, in_ss{in_ss}, ss{ss}, in_ts{in_ts}, ts{ts}, lhs{lhs} {}
        
        // Methods for writing easier to read for(...) loops
        bool is_in_S(int i) const;
        bool is_in_fs(int i) const;
        bool is_in_ss(int i) const;
        bool is_in_ts(int i) const;
        
        bool empty_S() const;
        int first_non_tabu() const;
    };
    
    sets_info   pi;
    sets_info   sigma;
    
    int         tot_number_of_iterations;
    int         tabu_duration;
    int         n;
        
    void update_info(sets_info& set, const sets_info& best, int bn, int iter);
    
    void add_pi_cut_if_violated(std::vector<IloRange>& cuts, const sets_info& pi);
    void add_sigma_cut_if_violated(std::vector<IloRange>& cuts, const sets_info& sigma);
    
    void add_groetschel_pi_cut_if_violated(std::vector<IloRange>& cuts, const sets_info& pi);
    void add_groetschel_sigma_cut_if_violated(std::vector<IloRange>& cuts, const sets_info& sigma);
    
    void add_or_remove_from_pi_sets(sets_info& pi, int i);
    void add_or_remove_from_sigma_sets(sets_info& sigma, int i);
    
    void recalculate_pi_sums(sets_info& pi);
    void recalculate_sigma_sums(sets_info& sigma);
    
    double calculate_groetschel_lhs_pi(const std::vector<int>& my_S, const sets_info& pi);
    double calculate_groetschel_lhs_sigma(const std::vector<int>& my_S, const sets_info& pi);
    
    void actually_add_groetschel_pi_cut(std::vector<IloRange>& cuts, const std::vector<int>& my_S, const sets_info& pi);
    void actually_add_groetschel_sigma_cut(std::vector<IloRange>& cuts, const std::vector<int>& my_S, const sets_info& sigma);
    
public:
    vi_separator_subtour_elimination(const tsp_graph& g, const program_params& params, const ch::solution& sol, const IloEnv& env, const IloNumVarArray& x, double eps);
    std::vector<IloRange> separate_valid_cuts();
};

#endif