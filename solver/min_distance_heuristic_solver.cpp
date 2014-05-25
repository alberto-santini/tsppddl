#ifndef MIN_DISTANCE_HEURISTIC_SOLVER_CPP
#define MIN_DISTANCE_HEURISTIC_SOLVER_CPP

#include <algorithm>

#include <solver/min_distance_heuristic_solver.h>

HeuristicSolution MinDistanceHeuristicSolver::solve(bool inverse_order) const {
    std::vector<int> path; path.reserve(2 * hh->n + 1);
    path.push_back(0); path.push_back(2 * hh->n + 1);
        
    std::vector<int> partial_load; partial_load.reserve(2 * hh->n + 1);
    partial_load.push_back(0); partial_load.push_back(0);
        
    int load = 0;
    double length = 0;
        
    std::vector<int> requests(hh->n);
    for(int i = 0; i < hh->n; i++) { requests[i] = i + 1; }
    
    std::sort(requests.begin(), requests.end(),
        [this, &inverse_order] (const int& r1, const int& r2) {
            bool greater = (hh->distance[r1][r1 + hh->n] > hh->distance[r2][r2 + hh->n]);
            return (inverse_order ? !greater : greater);
        }
    );
    
    // ``requests'' is sorted in REVERSE order, so we can easily pop_back()
    
    while(requests.size() != 0) {
        placement best_placement;

        double best_metric = 0;
        bool insertable = false;
                        
        for(int x = 1; x < path.size(); x++) {
            for(int y = x; y < path.size(); y++) {
                placement pl;
                bool feasible_and_improves;
                                    
                std::tie(feasible_and_improves, pl) = hh->new_path_if_feasible(x, y, requests.back(), length, load, best_metric, path, partial_load);
                        
                if(feasible_and_improves) {
                    insertable = true;
                    best_placement = pl;
                    best_metric = std::get<4>(pl);
                }
            }
        }
            
        if(!insertable) {
            return std::make_tuple(std::vector<int>(0), std::vector<int>(0), std::numeric_limits<int>::max());
        } else {
            path = std::get<0>(best_placement);
            partial_load = std::get<1>(best_placement);
            length = std::get<2>(best_placement);
            load = std::get<3>(best_placement);
            
            requests.pop_back();
        }
    }
    
    std::cout << "PATH GENERATED: ";
    for(const int& n_id : path) { std::cout << n_id << " "; }
    std::cout << std::endl << "COST: " << length << std::endl;
    
    double tested_length = hh->test_path_cost(path);
    std::cout << "RECOMPUTED COST: " << tested_length << std::endl;
    
    return std::make_tuple(path, partial_load, length);
}

#endif