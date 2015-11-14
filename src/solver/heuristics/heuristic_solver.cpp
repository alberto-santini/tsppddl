#include <solver/heuristics/heuristic_solver.h>

#include <heuristics/one_phase_heuristic.h>
#include <heuristics/two_phase_heuristic.h>
#include <heuristics/k_opt_heuristic.h>
#include <heuristics/path_scorer.h>
#include <heuristics/insertion_scorer.h>
#include <heuristics/request_scorer.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>

std::vector<path> heuristic_solver::run_constructive(bool print_output) {
    using namespace std::chrono;
    
    std::cout << "Heuristic solutions:         \t";
    
    std::ofstream results_file, summary_file, solutions_file;
    
    if(print_output) {
        results_file.open(params.ch.results_dir + "/results_details.txt", std::ios::out | std::ios::app);
        results_file << g.g[graph_bundle].instance_name << "\t";
    }
    
    // Path scorers
    ps_cost_opposite                                                    path_scorer_cost;
    ps_cost_plus_load_opposite                                          path_scorer_cost_plus_load;
    ps_load_times_cost_opposite                                         path_scorer_load_times_cost;
    ps_cost_times_capacity_usage                                        path_scorer_q_cost;
    
    // Insertion scorers
    insertion_scorer<decltype(path_scorer_cost)>                        insertion_scorer_cost(path_scorer_cost);
    insertion_scorer<decltype(path_scorer_cost_plus_load)>              insertion_scorer_cost_plus_load(path_scorer_cost_plus_load);
    insertion_scorer<decltype(path_scorer_load_times_cost)>             insertion_scorer_load_times_cost(path_scorer_load_times_cost);
    insertion_scorer<decltype(path_scorer_q_cost)>                      insertion_scorer_q_cost(path_scorer_q_cost);
    
    // Request scorers
    rs_origin_destination_distance                                      request_scorer_dist;
    rs_origin_destination_distance_opposite                             request_scorer_dist_opp;
    rs_draught_demand_difference                                        request_scorer_draught_demand;
    rs_draught_demand_difference_opposite                               request_scorer_draught_demand_opp;
    
    // Inserters (normal)
    normal_inserter<decltype(insertion_scorer_cost)>                    inserter_cost(insertion_scorer_cost);
    normal_inserter<decltype(insertion_scorer_cost_plus_load)>          inserter_cost_plus_load(insertion_scorer_cost_plus_load);
    normal_inserter<decltype(insertion_scorer_load_times_cost)>         inserter_load_times_cost(insertion_scorer_load_times_cost);
    normal_inserter<decltype(insertion_scorer_q_cost)>                  inserter_q_cost(insertion_scorer_q_cost);
    
    // Inserters (max regret)
    max_regret_inserter<decltype(insertion_scorer_cost)>                mr_inserter_cost(insertion_scorer_cost);
    max_regret_inserter<decltype(insertion_scorer_cost_plus_load)>      mr_inserter_cost_plus_load(insertion_scorer_cost_plus_load);
    max_regret_inserter<decltype(insertion_scorer_load_times_cost)>     mr_inserter_load_times_cost(insertion_scorer_load_times_cost);
    max_regret_inserter<decltype(insertion_scorer_q_cost)>              mr_inserter_q_cost(insertion_scorer_q_cost);
    
    // 1-phase heuristics (normal)
    one_phase_heuristic<decltype(inserter_cost)>                        oph_1(g, inserter_cost);
    one_phase_heuristic<decltype(inserter_cost_plus_load)>              oph_2(g, inserter_cost_plus_load);
    one_phase_heuristic<decltype(inserter_load_times_cost)>             oph_3(g, inserter_load_times_cost);
    one_phase_heuristic<decltype(inserter_q_cost)>                      oph_4(g, inserter_q_cost);
    
    // 1-phase heuristics (max-regret)
    one_phase_heuristic<decltype(mr_inserter_cost)>                     oph_5(g, mr_inserter_cost);
    one_phase_heuristic<decltype(mr_inserter_cost_plus_load)>           oph_6(g, mr_inserter_cost_plus_load);
    one_phase_heuristic<decltype(mr_inserter_load_times_cost)>          oph_7(g, mr_inserter_load_times_cost);
    one_phase_heuristic<decltype(mr_inserter_q_cost)>                   oph_8(g, mr_inserter_q_cost);
    
    // N.B. In 2-phase heuristics insertion_scorer_cost and insertion_scorer_load_times_cost are equivalent,
    // because once the sequence of requests is fixed, the total_load at each step is fixed, so only the cost part varies!
    // Same holds for insertion_scorer_cost_plus_load.
    
    // 2-phase heuristics
    two_phase_heuristic<
        decltype(request_scorer_dist),
        decltype(insertion_scorer_cost)>                                tph_1(g, request_scorer_dist, insertion_scorer_cost);
    two_phase_heuristic<
        decltype(request_scorer_dist),
        decltype(insertion_scorer_q_cost)>                              tph_2(g, request_scorer_dist, insertion_scorer_q_cost);
           
    two_phase_heuristic<
        decltype(request_scorer_dist_opp),
        decltype(insertion_scorer_cost)>                                tph_3(g, request_scorer_dist_opp, insertion_scorer_cost);
    two_phase_heuristic<
        decltype(request_scorer_dist_opp),
        decltype(insertion_scorer_q_cost)>                              tph_4(g, request_scorer_dist_opp, insertion_scorer_q_cost);
    
    two_phase_heuristic<
        decltype(request_scorer_draught_demand),
        decltype(insertion_scorer_cost)>                                tph_5(g, request_scorer_draught_demand, insertion_scorer_cost);
    two_phase_heuristic<
        decltype(request_scorer_draught_demand),
        decltype(insertion_scorer_q_cost)>                              tph_6(g, request_scorer_draught_demand, insertion_scorer_q_cost);
    
    two_phase_heuristic<
        decltype(request_scorer_draught_demand_opp),
        decltype(insertion_scorer_cost)>                                tph_7(g, request_scorer_draught_demand_opp, insertion_scorer_cost);
    two_phase_heuristic<
        decltype(request_scorer_draught_demand_opp),
        decltype(insertion_scorer_q_cost)>                              tph_8(g, request_scorer_draught_demand_opp, insertion_scorer_q_cost);
    
    
    #define TOKENPASTE(x, y) x ## y
    #define TOKENPASTE2(x, y) TOKENPASTE(x, y)
    
    #define HEUR_NAME(type, number) \
        TOKENPASTE2(TOKENPASTE2(type, _), number)

    #define HEUR_START_TIME(type, number) \
        TOKENPASTE2(t_start_, HEUR_NAME(type, number))

    #define HEUR_END_TIME(type, number) \
        TOKENPASTE2(t_end_, HEUR_NAME(type, number))

    #define HEUR_SPAN_TIME(type, number) \
        TOKENPASTE2(time_span_, HEUR_NAME(type, number))

    #define HEUR_RESULT(type, number) \
        TOKENPASTE2(result_, HEUR_NAME(type, number))

    #define EXECUTE_HEURISTIC(type, number) \
        auto HEUR_START_TIME(type, number) = high_resolution_clock::now(); \
        auto HEUR_RESULT(type, number) = HEUR_NAME(type, number).solve(); \
        auto HEUR_END_TIME(type, number) = high_resolution_clock::now(); \
        auto HEUR_SPAN_TIME(type, number) = duration_cast<duration<double>>(HEUR_END_TIME(type, number) - HEUR_START_TIME(type, number)); \
        data.time_spent_by_constructive_heuristics += HEUR_SPAN_TIME(type, number).count(); \
        \
        if(HEUR_RESULT(type, number)) { \
            std::cout << #type << "_" << #number << ":" << (*HEUR_RESULT(type, number)).total_cost << "\t"; \
            if(print_output) { \
                results_file << (*HEUR_RESULT(type, number)).total_cost << "\t"; \
            } \
            if((*HEUR_RESULT(type, number)).verify_feasible(g)) { \
                paths.push_back(*HEUR_RESULT(type, number)); \
            } else { \
                std::cout << "Generated path is not feasible!" << std::endl; \
                (*HEUR_RESULT(type, number)).print(std::cout); std::cout << std::endl; \
                if(print_output) { \
                    results_file << "iiii\t"; \
                } \
            } \
        } else { \
            std::cout << "xxxx\t"; \
            if(print_output) { \
                results_file << "xxxx\t"; \
            } \
        }

    EXECUTE_HEURISTIC(oph, 1)
    EXECUTE_HEURISTIC(oph, 2)
    EXECUTE_HEURISTIC(oph, 3)
    EXECUTE_HEURISTIC(oph, 4)
    EXECUTE_HEURISTIC(oph, 5)
    EXECUTE_HEURISTIC(oph, 6)
    EXECUTE_HEURISTIC(oph, 7)
    EXECUTE_HEURISTIC(oph, 8)
    
    EXECUTE_HEURISTIC(tph, 1)
    EXECUTE_HEURISTIC(tph, 2)
    EXECUTE_HEURISTIC(tph, 3)
    EXECUTE_HEURISTIC(tph, 4)
    EXECUTE_HEURISTIC(tph, 5)
    EXECUTE_HEURISTIC(tph, 6)
    EXECUTE_HEURISTIC(tph, 7)
    EXECUTE_HEURISTIC(tph, 8)
    
    std::cout << std::endl;
    
    data.n_constructive_solutions = paths.size();
    
    if(data.n_constructive_solutions > 0) {
        auto best_solution = std::min_element(paths.begin(), paths.end(), [] (const auto& p1, const auto& p2) { return p1.total_cost < p2.total_cost; });
        assert(best_solution != paths.end());
        data.best_constructive_solution = (*best_solution).total_cost;
    } else {
        data.best_constructive_solution = std::numeric_limits<double>::max();
    }
    
    if(print_output) {
        results_file << std::endl;
        results_file.close();
    
        summary_file.open(params.ch.results_dir + "/results.txt", std::ios::out | std::ios::app);
        
        summary_file << g.g[graph_bundle].instance_base_name << "\t";
        summary_file << g.g[graph_bundle].n << "\t";
        summary_file << g.g[graph_bundle].h << "\t";
        summary_file << g.g[graph_bundle].k << "\t";
        
        summary_file << data.best_constructive_solution << "\t";
        summary_file << data.time_spent_by_constructive_heuristics << std::endl;
        
        summary_file.close();
    
        solutions_file.open(params.ch.solutions_dir + "/" + g.g[graph_bundle].instance_name + ".txt", std::ios::out);
    
        for(auto i = 0u; i < paths.back().load_v.size(); i++) {
            for(auto p = 0u; p < paths.size(); p++) {
                solutions_file << paths.at(p).load_v.at(i) << "\t";
            }
            solutions_file << std::endl;
        }
    
        solutions_file.close();
    }
    
    return paths;
}

std::vector<path> heuristic_solver::run_k_opt() {
    auto appropriate_k_for_instance_size = 0;

    for(const auto& limit_pair : params.ko.instance_size_limits) {
         if(g.g[graph_bundle].n <= limit_pair.n && limit_pair.k > appropriate_k_for_instance_size) {
             appropriate_k_for_instance_size = limit_pair.k;
         }
    }

    auto h7 = k_opt_heuristic(g, params, data, appropriate_k_for_instance_size, paths);
    auto k_opt_paths = h7.solve(); // Time is counted within k_opt_heuristic

    std::cout << "Heuristic solutions:         \t";
    
    for(const auto& path : k_opt_paths) {
         std::cout << path.total_cost << "\t";
    }
    std::cout << std::endl;

    paths.insert(paths.end(), k_opt_paths.begin(), k_opt_paths.end());
    
    if(paths.size() > 0u) {
        auto best_solution = std::min_element(paths.begin(), paths.end(), [] (const auto& p1, const auto& p2) { return p1.total_cost < p2.total_cost; });
        assert(best_solution != paths.end());
        data.best_constructive_solution = (*best_solution).total_cost;
    } else {
        data.best_constructive_solution = std::numeric_limits<double>::max();
    }
    
    return paths;
}

std::vector<path> heuristic_solver::run_constructive_heuristics() {
    paths.clear();
    
    run_constructive(true);
    
    return paths;
}

std::vector<path> heuristic_solver::run_all_heuristics() {
    paths.clear();
    
    run_constructive(false);
    run_k_opt();
    
    return paths;
}