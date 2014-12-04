#include <cli/program.h>

#include <cstdlib>
#include <ctime>

namespace global {
    double g_total_time_spent_by_heuristics = 0;
    double g_total_time_spent_separating_cuts = 0;
    long g_total_number_of_feasibility_cuts_added = 0;
    long g_total_number_of_subtour_cuts_added = 0;
    long g_total_number_of_generalized_order_cuts_added = 0;
    long g_total_number_of_capacity_cuts_added = 0;
    long g_total_number_of_simplified_fork_cuts_addedd = 0;
}

int main(int argc, char* argv[]) {
    auto args = std::vector<std::string>(argv + 1, argv + argc);
    
    std::srand(123321); // For reproducibility
    
    auto p = Program();
    p.autorun(args);
    
    return 0;
}