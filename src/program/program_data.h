#ifndef PROGRAM_DATA_H
#define PROGRAM_DATA_H

struct program_data {
    double time_spent_by_constructive_heuristics;
    double time_spent_by_k_opt_heuristics;
    
    double time_spent_separating_feasibility_cuts;
    double time_spent_separating_subtour_elimination_vi;
    double time_spent_separating_generalised_order_vi;
    double time_spent_separating_capacity_vi;
    double time_spent_separating_simplified_fork_vi;
    double time_spent_separating_fork_vi;
    
    long total_number_of_feasibility_cuts_added;
    long total_number_of_subtour_elimination_vi_added;
    long total_number_of_generalised_order_vi_added;
    long total_number_of_capacity_vi_added;
    long total_number_of_simplified_fork_vi_added;
    long total_number_of_fork_vi_added;
    
    program_data() :
        time_spent_by_constructive_heuristics{0.0},
        time_spent_by_k_opt_heuristics{0.0},
        time_spent_separating_feasibility_cuts{0.0},
        time_spent_separating_subtour_elimination_vi{0.0},
        time_spent_separating_generalised_order_vi{0.0},
        time_spent_separating_capacity_vi{0.0},
        time_spent_separating_simplified_fork_vi{0.0},
        time_spent_separating_fork_vi{0.0},
        total_number_of_feasibility_cuts_added{0},
        total_number_of_subtour_elimination_vi_added{0},
        total_number_of_generalised_order_vi_added{0},
        total_number_of_capacity_vi_added{0},
        total_number_of_simplified_fork_vi_added{0},
        total_number_of_fork_vi_added{0} {}
        
    void reset_times_and_cuts();
};

#endif