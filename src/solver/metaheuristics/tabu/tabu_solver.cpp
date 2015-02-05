#include <solver/metaheuristics/tabu/kopt3_solver.h>
#include <solver/metaheuristics/tabu/tabu_solver.h>

#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

tabu_solver::tabu_solver(tsp_graph& g, const program_params& params, program_data& data, std::vector<path> initial_solutions, const std::string& instance_path) : g{g}, params{params}, data{data} {
    auto path_parts = std::vector<std::string>();
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    auto file_parts = std::vector<std::string>();
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));

    if(file_parts.size() > 1) {
        file_parts.pop_back();
    }

    instance_name = boost::algorithm::join(file_parts, ".");

    auto max_parallel_searches = std::min((unsigned int) params.ts.max_parallel_searches, (unsigned int) initial_solutions.size());
    
    std::partial_sort(
        initial_solutions.begin(),
        initial_solutions.begin() + max_parallel_searches,
        initial_solutions.end(),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.total_cost < rhs.total_cost;
        }
    );
    
    sliced_initial_solutions = std::vector<path>(initial_solutions.begin(), initial_solutions.begin() + max_parallel_searches);
}

std::vector<path> tabu_solver::solve() {
    auto solutions = std::vector<path>();
    auto threads = std::vector<std::thread>();
    std::mutex mtx;
    
    for(const auto& init_sol : sliced_initial_solutions) {
        threads.push_back(std::thread(
            [this, init_sol, &solutions, &mtx] () {
                auto solution = tabu_search(init_sol);
                
                std::lock_guard<std::mutex> guard(mtx);
                solutions.push_back(solution);
            }
        ));
    }
    
    using namespace std::chrono;
    
    auto t_start = high_resolution_clock::now();
    
    for(auto& t : threads) {
        t.join();
    }
    
    auto t_end = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_tabu_search = time_span.count();

    std::cout << "Metaheuristic solutions: \t";
    for(const auto& path : solutions) {
        std::cout << path.total_cost << "\t";
    }
    std::cout << std::endl;
    std::cout << "Solutions obtained in " << data.time_spent_by_tabu_search << " seconds." << std::endl;

    auto best_result = (*std::min_element(
            solutions.begin(),
            solutions.end(),
            [] (const auto& lhs, const auto& rhs) {
                return lhs.total_cost < rhs.total_cost;
            }
    )).total_cost;

    std::ofstream results_file;
    results_file.open(params.ts.results_dir + "results.txt", std::ios::out | std::ios::app);
    results_file << instance_name << "\t" << best_result << "\t" << data.time_spent_by_tabu_search << std::endl;
    results_file.close();

    return solutions;
}

path tabu_solver::tabu_search(path init_sol) {
    auto current_solution = init_sol;
    auto best_solution = init_sol;
    auto tabu_list = std::vector<tabu_move>();
    auto consecutive_not_improved = 0u;
    auto iterations = 0u;
    auto progress_report = std::map<unsigned int, int>();
    
    if(params.ts.track_progress) {
        progress_report.emplace(0u, init_sol.total_cost);
    }
    
    while(iterations < params.ts.max_iter && consecutive_not_improved < params.ts.max_iter_without_improving) {        
        auto kopt3solv = kopt3_solver(g);
        auto tabu_and_non_tabu = kopt3solv.solve(current_solution, tabu_list);

        auto overall_best_solution = tabu_and_non_tabu.overall_best;
        auto best_without_tabu_solution = tabu_and_non_tabu.best_without_tabu;

        if(overall_best_solution.empty()) {
            assert(best_without_tabu_solution.empty() && "Could not produce a general move but I have an halal move?!");
            
            std::cerr << "Tabu error: 3-opt solver could not produce any valid move!" << std::endl;
            return path();
        } else {
            if(overall_best_solution.p.total_cost < best_solution.total_cost - eps) {
                consecutive_not_improved = 0u;
                update_tabu_list(tabu_list, overall_best_solution);
                current_solution = overall_best_solution.p;
                best_solution = overall_best_solution.p;
            } else {
                consecutive_not_improved++;
                if(best_without_tabu_solution.empty()) {
                    update_tabu_list(tabu_list, overall_best_solution);
                    current_solution = overall_best_solution.p;
                } else {
                    update_tabu_list(tabu_list, best_without_tabu_solution);
                    current_solution = best_without_tabu_solution.p;
                }
            }
        }

        iterations++;
    }
    
    if(params.ts.track_progress) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 9999);
        
        std::stringstream file_name_ss;
        file_name_ss << params.ts.progress_results_dir << instance_name << "_" << dis(gen) << ".dat";
        
        std::ofstream progress_file;
        progress_file.open(file_name_ss.str(), std::ios::out | std::ios::app);
        
        for(const auto& prg : progress_report) {
            progress_file << prg.first << "\t" << prg.second << std::endl;
        }
        
        progress_file.close();
    }
    
    return best_solution;
}

void tabu_solver::update_tabu_list(std::vector<tabu_move>& tabu_list, const tabu_solver::tabu_result& sol) {
    if(std::find(tabu_list.begin(), tabu_list.end(), sol.shortest_erased_edge) == tabu_list.end()) {
        tabu_list.push_back(sol.shortest_erased_edge);
    }
    
    if(tabu_list.size() > params.ts.tabu_list_size) {
        tabu_list.erase(tabu_list.begin());
    }
}