#include <solver/heuristics/heuristic_solver.h>

#include <heuristics/best_insertion_heuristic.h>
#include <heuristics/heuristic_with_ordered_requests.h>
#include <heuristics/k_opt_heuristic.h>
#include <heuristics/max_regret_heuristic.h>

// #include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>

heuristic_solver::heuristic_solver(tsp_graph& g, const program_params& params, program_data& data, std::string instance_path) : g{g}, params{params}, data{data} {
    // PORTABLE WAY:
    // boost::filesystem::path i_path(instance_path);
    // std::stringstream ss;
    // ss << i_path.stem();
    // instance_name = ss.str();
    
    // NOT-PORTABLE WAY:
    auto path_parts = std::vector<std::string>();
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    auto file_parts = std::vector<std::string>();
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));
    
    if(file_parts.size() > 1) {
        file_parts.pop_back();
    }
    
    instance_name = boost::algorithm::join(file_parts, ".");
}

std::vector<path> heuristic_solver::run_constructive(bool print_output) {
    using namespace std::chrono;
    
    // Order insertions according to
    // (best_load / best_cost) > (new_load / new_cost)
    auto compare_load_over_cost = [] (int c1, int l1, int c2, int l2) -> bool {
        return ((double)l1 / (double) c1) > ((double)l2 / (double)c2);
    };
    
    // Order insertions according to
    // (best_load * best_cost) < (new_load * new_cost)
    auto compare_load_times_cost = [] (int c1, int l1, int c2, int l2) -> bool {
        if(l1 * c1 == 0) { return false; }
        if(l2 * c2 == 0) { return true; }
        return ((double)l1 * (double) c1) < ((double)l2 * (double)c2);
    };
    
    // Order insertions according to
    // best_cost < new_cost
    auto compare_cost = [] (int c1, int l1, int c2, int l2) -> bool {
        return (c1 < c2);
    };
    
    // Regret:
    // (best_load / best_cost) - (second_best_load / second_best_cost)
    auto regret_compare_load_over_cost = [] (int bc, int bl, int sbc, int sbl) -> double {
        return ((double)bl / (double)bc) - ((double)sbl / (double)sbc);
    };
    
    // Regret:
    // - (best_load * best_cost) + (second_best_load * second_best_cost)
    auto regret_compare_load_times_cost = [] (int bc, int bl, int sbc, int sbl) -> double {
        return abs((double)sbl * (double)sbc - (double)bl * (double)bc);
    };
    
    // Request comparator, request 1 is better than request 2 if
    // the distance between its origin and destination is SMALLER than for request 2
    auto compare_origin_destination_distance_small = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] < this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    // Request comparator, request 1 is better than request 2 if
    // the distance between its origin and destination is LARGER than for request 2
    auto compare_origin_destination_distance_large = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] > this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    // Sort the vector of requests according to (draught - demand) descending
    auto compare_draught_diff_desc = [this] (int r1, int r2) -> bool {
        auto n = this->g.g[graph_bundle].n;
        
        return (
            std::min(
                this->g.draught[r1] - std::abs(this->g.demand[r1]),
                this->g.draught[r1 + n] - std::abs(this->g.demand[r1 + n])
            ) <
            std::min(
                this->g.draught[r2] - std::abs(this->g.demand[r2]),
                this->g.draught[r2 + n] - std::abs(this->g.demand[r2 + n])
            )
        );
    };
    
    // Sort the vector of requests according to (draught - demand) ascending
    auto compare_draught_diff_asc = [this] (int r1, int r2) -> bool {
        auto n = this->g.g[graph_bundle].n;
        
        return (
            std::min(
                this->g.draught[r1] - std::abs(this->g.demand[r1]),
                this->g.draught[r1 + n] - std::abs(this->g.demand[r1 + n])
            ) >
            std::min(
                this->g.draught[r2] - std::abs(this->g.demand[r2]),
                this->g.draught[r2 + n] - std::abs(this->g.demand[r2 + n])
            )
        );
    };
    
    std::cout << "Heuristic solutions:         \t";
    
    std::ofstream results_file, solutions_file;
    
    if(print_output) {
        results_file.open(params.ch.results_dir + "/results.txt", std::ios::out | std::ios::app);
        results_file << instance_name << "\t";
    }
    
    //  CONSTRUCTIVE HEURISTICS
    
    auto h1 = max_regret_heuristic<decltype(compare_load_over_cost), decltype(regret_compare_load_over_cost)>(g, compare_load_over_cost, regret_compare_load_over_cost);
    auto t_start = high_resolution_clock::now();
    auto p1 = h1.solve();
    auto t_end = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p1) {
        std::cout << (*p1).total_cost << "\t";
        if(print_output) {
            results_file << (*p1).total_cost << "\t";
        }
        paths.push_back(*p1);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }

    auto h2 = max_regret_heuristic<decltype(compare_load_times_cost), decltype(regret_compare_load_times_cost)>(g, compare_load_times_cost, regret_compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p2 = h2.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p2) {
        std::cout << (*p2).total_cost << "\t";
        if(print_output) {
            results_file << (*p2).total_cost << "\t";
        }
        paths.push_back(*p2);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h3 = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_small), decltype(compare_cost)>(g, compare_origin_destination_distance_small, compare_cost);
    t_start = high_resolution_clock::now();
    auto p3 = h3.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p3) {
        std::cout << (*p3).total_cost << "\t";
        if(print_output) {
            results_file << (*p3).total_cost << "\t";
        }
        paths.push_back(*p3);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h4 = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_large), decltype(compare_cost)>(g, compare_origin_destination_distance_large, compare_cost);
    t_start = high_resolution_clock::now();
    auto p4 = h4.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p4) {
        std::cout << (*p4).total_cost << "\t";
        if(print_output) {
            results_file << (*p4).total_cost << "\t";
        }
        paths.push_back(*p4);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h7 = heuristic_with_ordered_requests<decltype(compare_draught_diff_desc), decltype(compare_cost)>(g, compare_draught_diff_desc, compare_cost);
    t_start = high_resolution_clock::now();
    auto p7 = h7.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p7) {
        std::cout << (*p7).total_cost << "\t";
        if(print_output) {
            results_file << (*p7).total_cost << "\t";
        }
        paths.push_back(*p7);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h8 = heuristic_with_ordered_requests<decltype(compare_draught_diff_asc), decltype(compare_cost)>(g, compare_draught_diff_asc, compare_cost);
    t_start = high_resolution_clock::now();
    auto p8 = h8.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p8) {
        std::cout << (*p8).total_cost << "\t";
        if(print_output) {
            results_file << (*p8).total_cost << "\t";
        }
        paths.push_back(*p8);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h3a = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_small), decltype(compare_load_over_cost)>(g, compare_origin_destination_distance_small, compare_load_over_cost);
    t_start = high_resolution_clock::now();
    auto p3a = h3a.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p3a) {
        std::cout << (*p3a).total_cost << "\t";
        if(print_output) {
            results_file << (*p3a).total_cost << "\t";
        }
        paths.push_back(*p3a);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h4a = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_large), decltype(compare_load_over_cost)>(g, compare_origin_destination_distance_large, compare_load_over_cost);
    t_start = high_resolution_clock::now();
    auto p4a = h4a.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p4a) {
        std::cout << (*p4a).total_cost << "\t";
        if(print_output) {
            results_file << (*p4a).total_cost << "\t";
        }
        paths.push_back(*p4a);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h7a = heuristic_with_ordered_requests<decltype(compare_draught_diff_desc), decltype(compare_load_over_cost)>(g, compare_draught_diff_desc, compare_load_over_cost);
    t_start = high_resolution_clock::now();
    auto p7a = h7a.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p7a) {
        std::cout << (*p7a).total_cost << "\t";
        if(print_output) {
            results_file << (*p7a).total_cost << "\t";
        }
        paths.push_back(*p7a);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h8a = heuristic_with_ordered_requests<decltype(compare_draught_diff_asc), decltype(compare_load_over_cost)>(g, compare_draught_diff_asc, compare_load_over_cost);
    t_start = high_resolution_clock::now();
    auto p8a = h8a.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p8a) {
        std::cout << (*p8a).total_cost << "\t";
        if(print_output) {
            results_file << (*p8a).total_cost << "\t";
        }
        paths.push_back(*p8a);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h3b = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_small), decltype(compare_load_times_cost)>(g, compare_origin_destination_distance_small, compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p3b = h3b.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p3b) {
        std::cout << (*p3b).total_cost << "\t";
        if(print_output) {
            results_file << (*p3b).total_cost << "\t";
        }
        paths.push_back(*p3b);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h4b = heuristic_with_ordered_requests<decltype(compare_origin_destination_distance_large), decltype(compare_load_times_cost)>(g, compare_origin_destination_distance_large, compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p4b = h4b.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p4b) {
        std::cout << (*p4b).total_cost << "\t";
        if(print_output) {
            results_file << (*p4b).total_cost << "\t";
        }
        paths.push_back(*p4b);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h7b = heuristic_with_ordered_requests<decltype(compare_draught_diff_desc), decltype(compare_load_times_cost)>(g, compare_draught_diff_desc, compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p7b = h7b.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p7b) {
        std::cout << (*p7b).total_cost << "\t";
        if(print_output) {
            results_file << (*p7b).total_cost << "\t";
        }
        paths.push_back(*p7b);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h8b = heuristic_with_ordered_requests<decltype(compare_draught_diff_asc), decltype(compare_load_times_cost)>(g, compare_draught_diff_asc, compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p8b = h8b.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p8b) {
        std::cout << (*p8b).total_cost << "\t";
        if(print_output) {
            results_file << (*p8b).total_cost << "\t";
        }
        paths.push_back(*p8b);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h5 = best_insertion_heuristic<decltype(compare_load_over_cost)>(g, compare_load_over_cost);
    t_start = high_resolution_clock::now();
    auto p5 = h5.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p5) {
        std::cout << (*p5).total_cost << "\t";
        if(print_output) {
            results_file << (*p5).total_cost << "\t";
        }
        paths.push_back(*p5);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    auto h6 = best_insertion_heuristic<decltype(compare_load_times_cost)>(g, compare_load_times_cost);
    t_start = high_resolution_clock::now();
    auto p6 = h6.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p6) {
        std::cout << (*p6).total_cost << "\t";
        if(print_output) {
            results_file << (*p6).total_cost << "\t";
        }
        paths.push_back(*p6);
    } else {
        std::cout << "xxxx\t";
        if(print_output) {
            results_file << "xxxx\t";
        }
    }
    
    std::cout << std::endl;
    
    if(print_output) {
        results_file << std::endl;
        results_file.close();
    
        solutions_file.open(params.ch.solutions_dir + "/" + instance_name + ".txt", std::ios::out);
    
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
    auto appropriate_k_for_instance_size = 0u;

    for(const auto& limit_pair : params.ko.instance_size_limits) {
         if((unsigned int) g.g[graph_bundle].n <= limit_pair.n && limit_pair.k > appropriate_k_for_instance_size) {
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