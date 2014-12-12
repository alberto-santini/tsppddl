#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <iostream>
#include <string>

struct k_opt_limit {
    unsigned int k;
    unsigned int n;
    
    k_opt_limit(unsigned int k, unsigned int n) : k{k}, n{n} {}
};

using k_opt_limits = std::vector<k_opt_limit>;

struct KOptParams {
    k_opt_limits instance_size_limits;
    
    KOptParams(k_opt_limits instance_size_limits) : instance_size_limits{std::move(instance_size_limits)} {}
};

struct SubgradientParams {
    bool            relax_mtz;
    bool            relax_prec;
    double          initial_lambda;
    double          initial_mu;
    unsigned int    iter_reduce_theta;
    double          theta_reduce_factor;
    unsigned int    max_iter;
    std::string     results_dir;
    
    SubgradientParams(  bool relax_mtz,
                        bool relax_prec,
                        double initial_lambda,
                        double initial_mu,
                        unsigned int iter_reduce_theta,
                        double theta_reduce_factor,
                        unsigned int max_iter,
                        std::string results_dir) :
                        relax_mtz{relax_mtz},
                        relax_prec{relax_prec},
                        initial_lambda{initial_lambda},
                        initial_mu{initial_mu},
                        iter_reduce_theta{iter_reduce_theta},
                        theta_reduce_factor{theta_reduce_factor},
                        max_iter{max_iter},
                        results_dir{std::move(results_dir)} {}
};

struct BranchAndCutParams {
    unsigned int    cut_every_n_nodes;
    bool            two_cycles_elim;
    bool            subpath_elim;
    bool            subtour_sep_memory;
    bool            separate_subtour_elimination;
    bool            separate_precedence;
    bool            separate_capacity;
    bool            separate_simplified_fork;
    bool            print_relaxation_graph;
    std::string     results_dir;
    
    BranchAndCutParams( unsigned int cut_every_n_nodes,
                        bool two_cycles_elim,
                        bool subpath_elim,
                        bool subtour_sep_memory,
                        bool separate_subtour_elimination,
                        bool separate_precedence,
                        bool separate_capacity,
                        bool separate_simplified_fork,
                        bool print_relaxation_graph,
                        std::string results_dir) :
                        cut_every_n_nodes{cut_every_n_nodes},
                        two_cycles_elim{two_cycles_elim},
                        subpath_elim{subpath_elim},
                        subtour_sep_memory{subtour_sep_memory},
                        separate_subtour_elimination{separate_subtour_elimination},
                        separate_precedence{separate_precedence},
                        separate_capacity{separate_capacity},
                        separate_simplified_fork{separate_simplified_fork},
                        print_relaxation_graph{print_relaxation_graph},
                        results_dir{std::move(results_dir)} {
        if(separate_simplified_fork) {
            std::cout << "WARNING: separate_simplified_fork is deprecated, as it doesn't do much. All the cuts it could separate are already enumerated by subpath_elim (if you have it on) or very likely separated by the feasibility cut generator (as they concern precedence). Even with subpath_elim off, in our instance is almost never the case that separate_simplified_fork can find a violated cut." << std::endl;
        }
    }
};

struct ProgramParams {
    KOptParams          ko;
    SubgradientParams   sg;
    BranchAndCutParams  bc;
    
    ProgramParams(KOptParams ko, SubgradientParams sg, BranchAndCutParams bc) : ko{std::move(ko)}, sg{std::move(sg)}, bc{std::move(bc)} {}
};

#endif