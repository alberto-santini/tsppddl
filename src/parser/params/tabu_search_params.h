#ifndef TABU_SEARCH_PARAMS_H
#define TABU_SEARCH_PARAMS_H

#include <string>
#include <utility>

struct tabu_search_params {
    int         tabu_list_size;
    int         max_iter;
    int         max_iter_without_improving;
    int         max_parallel_searches;
    std::string results_dir;
    bool        track_progress;
    std::string progress_results_dir;
    
    tabu_search_params() {}
    tabu_search_params( int tabu_list_size,
                        int max_iter,
                        int max_iter_without_improving,
                        int max_parallel_searches,
                        std::string results_dir,
                        bool track_progress,
                        std::string progress_results_dir) : 
                        tabu_list_size{tabu_list_size},
                        max_iter{max_iter},
                        max_iter_without_improving{max_iter_without_improving},
                        max_parallel_searches{max_parallel_searches},
                        results_dir{results_dir},
                        track_progress{track_progress},
                        progress_results_dir{progress_results_dir} {}
};

struct tabu_search_tuning_params {
    std::vector<int> tabu_list_size;
    
    tabu_search_tuning_params() {}
    tabu_search_tuning_params(std::vector<int> tabu_list_size) : tabu_list_size{tabu_list_size} {}
};

#endif