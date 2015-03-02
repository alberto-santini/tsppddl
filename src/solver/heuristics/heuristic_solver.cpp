#include <solver/heuristics/heuristic_solver.h>

#include <heuristics/best_insertion_heuristic.h>
#include <heuristics/heuristic_with_ordered_requests.h>
#include <heuristics/k_opt_heuristic.h>
#include <heuristics/max_regret_heuristic.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <ratio>

std::vector<path> heuristic_solver::run_constructive() {
    using namespace std::chrono;
    
    // Drop a request (and don't insert it) when the ratio
    // (best_load / best_cost) > (new_load / new_cost)
    auto ic1 = [] (int c1, int l1, int c2, int l2) -> bool {
        return ((double)l1 / (double) c1) > ((double)l2 / (double)c2);
    };
    
    // Drop a request (and don't insert it) when the ratio
    // (best_load * best_cost) < (new_load * new_cost)
    auto ic2 = [] (int c1, int l1, int c2, int l2) -> bool {
        if(l1 * c1 == 0) { return false; }
        if(l2 * c2 == 0) { return true; }
        return ((double)l1 * (double) c1) < ((double)l2 * (double)c2);
    };
    
    // Drop a request (and don't insert it) when
    // best_cost < new_cost
    auto ic3 = [] (int c1, int l1, int c2, int l2) -> bool {
        return (c1 < c2);
    };
    
    // Regret:
    // (best_load / best_cost) - (second_best_load / second_best_cost)
    auto rg1 = [] (int bc, int bl, int sbc, int sbl) -> double {
        return ((double)bl / (double)bc) - ((double)sbl / (double)sbc);
    };
    
    // Regret:
    // - (best_load * best_cost) + (second_best_load * second_best_cost)
    auto rg2 = [] (int bc, int bl, int sbc, int sbl) -> double {
        return abs((double)sbl * (double)sbc - (double)bl * (double)bc);
    };
    
    // Request comparator, request 1 is better than request 2 if
    // the distance between its origin and destination is SMALLER than for request 2
    auto rc1 = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] < this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    // Request comparator, request 1 is better than request 2 if
    // the distance between its origin and destination is LARGER than for request 2
    auto rc2 = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] > this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    auto current_time = std::time(nullptr);
    auto local_time = *std::localtime(&current_time);
    
    // std::put_time not implemented as of GCC 4.9.2
    // std::cout << std::put_time(&local_time, "%H-%M-%S") << " Heuristic solutions:         \t";
    char unbealivable_i_have_to_do_this_gcc_wtf[100];
    if(std::strftime(unbealivable_i_have_to_do_this_gcc_wtf, sizeof(unbealivable_i_have_to_do_this_gcc_wtf), "%H-%M-%S", &local_time)) {
        std::cout << unbealivable_i_have_to_do_this_gcc_wtf << " Heuristic solutions:         \t";
    }
    
    // Sort the vector of requests so that those with a bigger (draught - demand) value are
    // in the middle, while the "worse" ones are either at the beginning or at the end where
    // given the low cargo on board, not so much flexibility is required
    struct sort_with_low_draught_first_or_last {
        bool is_custom = true;
        const tsp_graph& g;
        int n;
        
        sort_with_low_draught_first_or_last(const tsp_graph& g) : g{g}, n{g.g[graph_bundle].n} {}
        
        void sort_in_place(std::vector<int>& remaining_requests) const {
            auto parity = false;
            auto req_list = std::list<int>();
            
            std::sort(remaining_requests.begin(), remaining_requests.end(),
                [this] (int r1, int r2) -> bool {
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
                }
            );
    
            for(auto req : remaining_requests) {
                if(parity) {
                    req_list.push_front(req);
                } else {
                    req_list.push_back(req);
                }
    
                parity = !parity;
            }

            remaining_requests = std::vector<int>(req_list.begin(), req_list.end());
        }
    };
    
    auto low_draught_sorter = sort_with_low_draught_first_or_last(g);
    
    //  CONSTRUCTIVE HEURISTICS
    
    auto h1 = max_regret_heuristic<decltype(ic1), decltype(rg1)>(g, ic1, rg1);
    auto t_start = high_resolution_clock::now();
    auto p1 = h1.solve();
    auto t_end = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p1) {
        std::cout << (*p1).total_cost << "\t";
        paths.push_back(*p1);
    } else {
        std::cout << "xxxx\t";
    }

    auto h2 = max_regret_heuristic<decltype(ic2), decltype(rg2)>(g, ic2, rg2);
    t_start = high_resolution_clock::now();
    auto p2 = h2.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p2) {
        std::cout << (*p2).total_cost << "\t";
        paths.push_back(*p2);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h3 = heuristic_with_ordered_requests<decltype(rc1), decltype(ic3)>(g, rc1, ic3);
    t_start = high_resolution_clock::now();
    auto p3 = h3.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p3) {
        std::cout << (*p3).total_cost << "\t";
        paths.push_back(*p3);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h4 = heuristic_with_ordered_requests<decltype(rc2), decltype(ic3)>(g, rc2, ic3);
    t_start = high_resolution_clock::now();
    auto p4 = h4.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p4) {
        std::cout << (*p4).total_cost << "\t";
        paths.push_back(*p4);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h5 = best_insertion_heuristic<decltype(ic1)>(g, ic1);
    t_start = high_resolution_clock::now();
    auto p5 = h5.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p5) {
        std::cout << (*p5).total_cost << "\t";
        paths.push_back(*p5);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h6 = best_insertion_heuristic<decltype(ic2)>(g, ic2);
    t_start = high_resolution_clock::now();
    auto p6 = h6.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p6) {
        std::cout << (*p6).total_cost << "\t";
        paths.push_back(*p6);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h7 = heuristic_with_ordered_requests<decltype(rc1), decltype(ic3), sort_with_low_draught_first_or_last>(g, rc1, ic3, low_draught_sorter);
    t_start = high_resolution_clock::now();
    auto p7 = h7.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p7) {
        std::cout << (*p7).total_cost << "\t";
        paths.push_back(*p7);
    } else {
        std::cout << "xxxx\t";
    }
    
    auto h8 = heuristic_with_ordered_requests<decltype(rc2), decltype(ic3), sort_with_low_draught_first_or_last>(g, rc2, ic3, low_draught_sorter);
    t_start = high_resolution_clock::now();
    auto p8 = h8.solve();
    t_end = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t_end - t_start);
    data.time_spent_by_constructive_heuristics += time_span.count();
    
    if(p8) {
        std::cout << (*p8).total_cost << "\t";
        paths.push_back(*p8);
    } else {
        std::cout << "xxxx\t";
    }
    
    std::cout << std::endl;
    
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

    auto current_time = std::time(nullptr);
    auto local_time = *std::localtime(&current_time);
    
    // std::put_time not implemented as of GCC 4.9.2
    // std::cout << std::put_time(&local_time, "%H-%M-%S") << " Heuristic solutions:         \t";
    char unbealivable_i_have_to_do_this_gcc_wtf[100];
    if(std::strftime(unbealivable_i_have_to_do_this_gcc_wtf, sizeof(unbealivable_i_have_to_do_this_gcc_wtf), "%H-%M-%S", &local_time)) {
        std::cout << unbealivable_i_have_to_do_this_gcc_wtf << " Heuristic solutions:         \t";
    }
    
    for(const auto& path : k_opt_paths) {
         std::cout << path.total_cost << "\t";
    }
    std::cout << std::endl;

    paths.insert(paths.end(), k_opt_paths.begin(), k_opt_paths.end());
    
    return paths;
}

std::vector<path> heuristic_solver::run_all() {
    paths.clear();
    
    run_constructive();
    run_k_opt();
    
    return paths;
}