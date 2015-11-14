#ifndef SUBGRADIENT_PARAMS_H
#define SUBGRADIENT_PARAMS_H

#include <string>

struct subgradient_params {
    bool            relax_mtz;
    bool            relax_prec;
    double          initial_lambda;
    double          initial_mu;
    int             iter_reduce_theta;
    double          theta_reduce_factor;
    int             max_iter;
    std::string     results_dir;
    
    subgradient_params() {}
    subgradient_params(  bool relax_mtz,
                        bool relax_prec,
                        double initial_lambda,
                        double initial_mu,
                        int iter_reduce_theta,
                        double theta_reduce_factor,
                        int max_iter,
                        std::string results_dir) :
                        relax_mtz{relax_mtz},
                        relax_prec{relax_prec},
                        initial_lambda{initial_lambda},
                        initial_mu{initial_mu},
                        iter_reduce_theta{iter_reduce_theta},
                        theta_reduce_factor{theta_reduce_factor},
                        max_iter{max_iter},
                        results_dir{results_dir} {}
};

#endif