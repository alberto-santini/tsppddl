#ifndef TABU_SEARCH_PARAMS_H
#define TABU_SEARCH_PARAMS_H

#include <string>
#include <utility>

struct tabu_search_params {
    unsigned int    tabu_list_size;
    unsigned int    max_iter;
    unsigned int    max_iter_without_improving;
    unsigned int    max_parallel_searches;
    std::string     results_dir;
    
    tabu_search_params() {}
    tabu_search_params(unsigned int tabu_list_size, unsigned int max_iter, unsigned int max_iter_without_improving, unsigned int max_parallel_searches, std::string results_dir) : tabu_list_size{tabu_list_size}, max_iter{max_iter}, max_iter_without_improving{max_iter_without_improving}, max_parallel_searches{max_parallel_searches}, results_dir{std::move(results_dir)} {}
};

#endif