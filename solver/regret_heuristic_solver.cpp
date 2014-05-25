#ifndef REGRET_HEURISTIC_SOLVER_CPP
#define REGRET_HEURISTIC_SOLVER_CPP

#include <algorithm>
#include <limits>
#include <iostream>

#include <solver/regret_heuristic_solver.h>

HeuristicSolution RegretHeuristicSolver::solve() const {
    std::vector<int> path; path.reserve(2 * hh->n + 1);
    path.push_back(0); path.push_back(2 * hh->n + 1);
        
    std::vector<int> partial_load; partial_load.reserve(2 * hh->n + 1);
    partial_load.push_back(0); partial_load.push_back(0);
        
    int load = 0;
    double length = 0;
    
    std::vector<int> remaining(hh->n);
    for(int i = 0; i < hh->n; i++) { remaining[i] = i + 1; }
        
    while(remaining.size() != 0) {
        // Map:
        // request_id => placement
        // Placement:
        // (new_path, new_partial_load, new_length, new_load, new_metric)
        std::map<int, placement> best_placement;
        std::map<int, placement> second_best_placement;
        
        for(const int& i : remaining) {
            double best_metric = 0;
            double second_best_metric = 0;
            bool insertable = false;
            
            for(int x = 1; x < path.size(); x++) {
                for(int y = x; y < path.size(); y++) {
                    placement pl;
                    bool feasible_and_improves;
                        
                    std::tie(feasible_and_improves, pl) = hh->new_path_if_feasible(x, y, i, length, load, second_best_metric, path, partial_load);
                        
                    if(feasible_and_improves) {
                        insertable = true;
                        if(std::get<4>(pl) > best_metric) {
                            second_best_placement[i] = best_placement[i];
                            second_best_metric = best_metric;
                            best_placement[i] = pl;
                            best_metric = std::get<4>(pl);
                        } else {
                            second_best_placement[i] = pl;
                            second_best_metric = std::get<4>(pl);
                        }
                    }
                }
            }
            
            if(!insertable) {
                best_placement[i] = std::make_tuple(std::vector<int>(0), std::vector<int>(0), std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::max());
                second_best_placement[i] = std::make_tuple(std::vector<int>(0), std::vector<int>(0), std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::max());
            }
        }
                
        int req_with_greatest_diff = remaining[0];
        double greatest_diff = std::get<4>(best_placement[remaining[0]]) - std::get<4>(second_best_placement[remaining[0]]);
        placement pl = best_placement[remaining[0]];
        
        for(auto rem_it = remaining.begin() + 1; rem_it != remaining.end(); ++rem_it) {
            double diff = std::get<4>(best_placement[*rem_it]) - std::get<4>(second_best_placement[*rem_it]);
            
            if(diff > greatest_diff) {
                req_with_greatest_diff = *rem_it;
                greatest_diff = diff;
                pl = best_placement[*rem_it];
            }
        }
        
        if(greatest_diff == 0 && std::get<4>(pl) == 0) {
            return std::make_tuple(std::vector<int>(0), std::vector<int>(0), std::numeric_limits<int>::max());
        }
        
        path = std::get<0>(pl);
        partial_load = std::get<1>(pl);
        length = std::get<2>(pl);
        load = std::get<3>(pl);
        
        remaining.erase(std::remove(remaining.begin(), remaining.end(), req_with_greatest_diff), remaining.end());
    }
    
    std::cout << "PATH GENERATED: ";
    for(const int& n_id : path) { std::cout << n_id << " "; }
    std::cout << std::endl << "COST: " << length << std::endl;
    
    double tested_length = hh->test_path_cost(path);
    std::cout << "RECOMPUTED COST: " << tested_length << std::endl;
    
    return std::make_tuple(path, partial_load, length);
}

#endif