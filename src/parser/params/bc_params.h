#ifndef BC_PARAMS_H
#define BC_PARAMS_H

#include <iostream>
#include <string>

struct branch_and_cut_params {
    bool            two_cycles_elim;
    bool            subpath_elim;
    unsigned int    max_infeas_subpaths;
    bool            print_relaxation_graph;
    bool            use_initial_solutions;
    std::string     results_dir;
    
    struct valid_inequality_info {
        unsigned int    cut_every_n_nodes;
        bool            enabled;
        
        static const unsigned int unreachable_number_of_bb_nodes = 1e07;
    
        valid_inequality_info() {}
        valid_inequality_info(unsigned int cut_every_n_nodes, bool enabled) : cut_every_n_nodes{cut_every_n_nodes}, enabled{enabled} {
            if(cut_every_n_nodes == 0u) {
                std::cerr << "bc_params.h::valid_inequality_info() \t WARNING: value of 0 for cut_every_n_nodes means `only when there is an integer solution'" << std::endl;
                this->cut_every_n_nodes = unreachable_number_of_bb_nodes;
            }
        }
    };

    struct valid_inequality_with_memory_info : public valid_inequality_info {
        bool memory;
    
        valid_inequality_with_memory_info() {}
        valid_inequality_with_memory_info(unsigned int cut_every_n_nodes, bool enabled, bool memory) : valid_inequality_info{cut_every_n_nodes, enabled}, memory{memory} {}
    };
    
    struct valid_inequality_with_lifted_version_info : public valid_inequality_info {
        bool lifted;
        
        valid_inequality_with_lifted_version_info() {}
        valid_inequality_with_lifted_version_info(unsigned int cut_every_n_nodes, bool enabled, bool lifted) : valid_inequality_info{cut_every_n_nodes, enabled}, lifted{lifted} {}
    };
    
    valid_inequality_with_memory_info subtour_elim;
    valid_inequality_info generalised_order, capacity;
    valid_inequality_with_lifted_version_info fork;
    
    branch_and_cut_params() {}
    branch_and_cut_params(  bool two_cycles_elim,
                            bool subpath_elim,
                            unsigned int max_infeas_subpaths,
                            bool print_relaxation_graph,
                            bool use_initial_solutions,
                            std::string results_dir,
                            valid_inequality_with_memory_info subtour_elim,
                            valid_inequality_info feasibility_cuts,
                            valid_inequality_info generalised_order,
                            valid_inequality_info capacity,
                            valid_inequality_with_lifted_version_info fork) :
                            two_cycles_elim{two_cycles_elim},
                            subpath_elim{subpath_elim},
                            max_infeas_subpaths{max_infeas_subpaths},
                            print_relaxation_graph{print_relaxation_graph},
                            use_initial_solutions{use_initial_solutions},
                            results_dir{std::move(results_dir)},
                            subtour_elim{std::move(subtour_elim)},
                            feasibility_cuts{std::move(feasibility_cuts)},
                            generalised_order{std::move(generalised_order)},
                            capacity{std::move(capacity)},
                            fork{std::move(fork)} {
        if(simplified_fork.enabled) {
        }
        if(feasibility_cuts.cut_every_n_nodes != 1u) {
            std::cerr << "bc_params.h::branch_and_cut_params() \t WARNING: you are not separating feasibility cuts at every node. This will likely produce infeasible solutions! Only do this if you know what you are doing!" << std::endl;
        }
    }
};

#endif