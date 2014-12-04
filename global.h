#ifndef GLOBAL_H
#define GLOBAL_H

namespace global {
    extern double g_total_time_spent_by_heuristics;
    extern double g_total_time_spent_separating_cuts;
    extern long g_total_number_of_feasibility_cuts_added;
    extern long g_total_number_of_subtour_cuts_added;
    extern long g_total_number_of_generalized_order_cuts_added;
    extern long g_total_number_of_capacity_cuts_added;
    extern long g_total_number_of_simplified_fork_cuts_addedd;
}

#endif