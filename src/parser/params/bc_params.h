#ifndef BC_PARAMS_H
#define BC_PARAMS_H

#include <iostream>
#include <string>

struct branch_and_cut_params {
    bool            two_cycles_elim;
    bool            subpath_elim;
    unsigned int    max_infeas_subpaths;
    bool            print_relaxation_graph;
    std::string     results_dir;
    
    struct valid_inequality_info {
        unsigned int    cut_every_n_nodes;
        bool            enabled;
    
        valid_inequality_info() {}
        valid_inequality_info(unsigned int cut_every_n_nodes, bool enabled) : cut_every_n_nodes{cut_every_n_nodes}, enabled{enabled} {}
    };

    struct valid_inequality_with_memory_info : public valid_inequality_info {
        bool            memory;
    
        valid_inequality_with_memory_info() {}
        valid_inequality_with_memory_info(unsigned int cut_every_n_nodes, bool enabled, bool memory) : valid_inequality_info{cut_every_n_nodes, enabled}, memory{memory} {}
    };
    
    valid_inequality_with_memory_info subtour_elim;
    valid_inequality_info feasibility_cuts, generalised_order, capacity, simplified_fork, fork;
    
    branch_and_cut_params() {}
    branch_and_cut_params(  bool two_cycles_elim,
                            bool subpath_elim,
                            unsigned int max_infeas_subpaths,
                            bool print_relaxation_graph,
                            std::string results_dir,
                            valid_inequality_with_memory_info subtour_elim,
                            valid_inequality_info feasibility_cuts,
                            valid_inequality_info generalised_order,
                            valid_inequality_info capacity,
                            valid_inequality_info simplified_fork,
                            valid_inequality_info fork) :
                            two_cycles_elim{two_cycles_elim},
                            subpath_elim{subpath_elim},
                            max_infeas_subpaths{max_infeas_subpaths},
                            print_relaxation_graph{print_relaxation_graph},
                            results_dir{std::move(results_dir)},
                            subtour_elim{std::move(subtour_elim)},
                            feasibility_cuts{std::move(feasibility_cuts)},
                            generalised_order{std::move(generalised_order)},
                            capacity{std::move(capacity)},
                            simplified_fork{std::move(simplified_fork)},
                            fork{std::move(fork)} {
        if(simplified_fork.enabled) {
            std::cerr << "bc_params.h::branch_and_cut_params() \t WARNING: simplified_fork cuts are deprecated, as they don't do much. All these cuts are already enumerated by subpath_elim (if you have it on) or very likely separated by the feasibility cut generator (as they concern generalised_order). Even with subpath_elim off, in our instances it's almost never the case that separate_simplified_fork can find a violated cut." << std::endl;
        }
        if(feasibility_cuts.cut_every_n_nodes != 1u) {
            std::cerr << "bc_params.h::branch_and_cut_params() \t WARNING: you are not separating feasibility cuts at every node. This will likely produce infeasible solutions! Only do this if you know what you are doing!" << std::endl;
        }
    }
};

#endif