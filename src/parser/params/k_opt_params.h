#ifndef K_OPT_PARAMS_H
#define K_OPT_PARAMS_H

struct k_opt_params {
    struct k_opt_limit {
        int k;
        int n;

        k_opt_limit() {}
        k_opt_limit(int k, int n) : k{k}, n{n} {}
    };
    
    using k_opt_limits = std::vector<k_opt_limit>;
    
    k_opt_limits instance_size_limits;
    
    k_opt_params() {}
    k_opt_params(k_opt_limits instance_size_limits) : instance_size_limits{instance_size_limits} {}
};

#endif