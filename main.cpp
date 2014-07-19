#include <cli/program.h>

#include <iostream>

double g_total_time_spent_by_heuristics {0};
double g_total_time_spent_separating_cuts {0};
long g_total_number_of_cuts_added {0};
long g_search_for_cuts_every_n_nodes {100};

int main(int argc, char* argv[]) {
    if(argc != 4) {
        std::cout << "Usage: tsppddl <instance_name> <cuts_every_n_nodes> <use_valid_y_ineq:true|false>" << std::endl;
        return -1;
    }
    
    std::vector<std::string> args(argv + 1, argv + argc);
    
    Program p;
    
    // p.prompt();
    
    p.autorun(args);
    
    return 0;
}