#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <string>

struct SubgradientParams {
    bool            relax_mtz;
    bool            relax_prec;
    double          initial_lambda;
    double          initial_mu;
    unsigned int    iter_reduce_theta;
    double          theta_reduce_factor;
    unsigned int    max_iter;
    std::string     results_dir;
    
    SubgradientParams(bool relax_mtz, bool relax_prec, double initial_lambda, double initial_mu, unsigned int iter_reduce_theta, double theta_reduce_factor, unsigned int max_iter, std::string results_dir) : relax_mtz{relax_mtz}, relax_prec{relax_prec}, initial_lambda{initial_lambda}, initial_mu{initial_mu}, iter_reduce_theta{iter_reduce_theta}, theta_reduce_factor{theta_reduce_factor}, max_iter{max_iter}, results_dir{std::move(results_dir)} {}
};

struct BranchAndCutParams {
    unsigned int    cut_every_n_nodes;
    bool            two_cycles_elim;
    bool            subpath_elim;
    bool            subtour_sep_memory;
    bool            separate_subtour_elimination;
    bool            separate_precedence;
    bool            separate_capacity;
    std::string     results_dir;
    
    BranchAndCutParams( unsigned int cut_every_n_nodes,
                        bool two_cycles_elim,
                        bool subpath_elim,
                        bool subtour_sep_memory,
                        bool separate_subtour_elimination,
                        bool separate_precedence,
                        bool separate_capacity,
                        std::string results_dir) :
                        cut_every_n_nodes{cut_every_n_nodes},
                        two_cycles_elim{two_cycles_elim},
                        subpath_elim{subpath_elim},
                        subtour_sep_memory{subtour_sep_memory},
                        separate_subtour_elimination{separate_subtour_elimination},
                        separate_precedence{separate_precedence},
                        separate_capacity{separate_capacity},
                        results_dir{std::move(results_dir)} {}
};

struct ProgramParams {
    SubgradientParams   sg;
    BranchAndCutParams  bc;
    
    ProgramParams(SubgradientParams sg, BranchAndCutParams bc) : sg{std::move(sg)}, bc{std::move(bc)} {}
};

#endif