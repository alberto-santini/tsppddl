#ifndef BC_PARAMS_H
#define BC_PARAMS_H

#include <iostream>
#include <string>

struct branch_and_cut_params {
    bool            two_cycles_elim;
    bool            subpath_elim;
    int             max_infeas_subpaths;
    bool            print_relaxation_graph;
    bool            use_initial_solutions;
    std::string     results_dir;
    
    struct valid_inequality_info {
        int             n1;
        int             n2;
        double          p1;
        double          p2;
        double          p3;
        bool            enabled;
        
        static const int unreachable_number_of_bb_nodes = 1e07;
    
        valid_inequality_info() {}
        valid_inequality_info(int n1, int n2, double p1, double p2, double p3, bool enabled) : n1{n1}, n2{n2}, p1{p1}, p2{p2}, p3{p3}, enabled{enabled} {}
    };

    struct valid_inequality_with_memory_info : public valid_inequality_info {
        bool memory;
    
        valid_inequality_with_memory_info() {}
        valid_inequality_with_memory_info(int n1, int n2, double p1, double p2, double p3, bool enabled, bool memory) : valid_inequality_info{n1, n2, p1, p2, p3, enabled}, memory{memory} {}
    };
    
    struct valid_inequality_with_lifted_version_info : public valid_inequality_info {
        bool lifted;
        
        valid_inequality_with_lifted_version_info() {}
        valid_inequality_with_lifted_version_info(int n1, int n2, double p1, double p2, double p3, bool enabled, bool lifted) : valid_inequality_info{n1, n2, p1, p2, p3, enabled}, lifted{lifted} {}
    };
    
    valid_inequality_with_memory_info subtour_elim;
    valid_inequality_info generalised_order, capacity;
    valid_inequality_with_lifted_version_info fork;
    
    branch_and_cut_params() {}
    branch_and_cut_params(  bool two_cycles_elim,
                            bool subpath_elim,
                            int max_infeas_subpaths,
                            bool print_relaxation_graph,
                            bool use_initial_solutions,
                            std::string results_dir,
                            valid_inequality_with_memory_info subtour_elim,
                            valid_inequality_info generalised_order,
                            valid_inequality_info capacity,
                            valid_inequality_with_lifted_version_info fork) :
                            two_cycles_elim{two_cycles_elim},
                            subpath_elim{subpath_elim},
                            max_infeas_subpaths{max_infeas_subpaths},
                            print_relaxation_graph{print_relaxation_graph},
                            use_initial_solutions{use_initial_solutions},
                            results_dir{results_dir},
                            subtour_elim{subtour_elim},
                            generalised_order{generalised_order},
                            capacity{capacity},
                            fork{fork} {}
};

#endif