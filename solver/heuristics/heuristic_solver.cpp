#include <solver/heuristics/heuristic_solver.h>

#include <heuristics/best_insertion_heuristic.h>
#include <heuristics/heuristic_with_ordered_requests.h>
#include <heuristics/k_opt_heuristic.h>
#include <heuristics/max_regret_heuristic.h>

#include <iostream>

std::vector<path> heuristic_solver::solve() const {
    auto paths = std::vector<path>();
        
    auto ic1 = [] (int c1, int l1, int c2, int l2) -> bool {
        return ((double)l1 / (double) c1) > ((double)l2 / (double)c2);
    };
    
    auto ic2 = [] (int c1, int l1, int c2, int l2) -> bool {
        if(l1 * c1 == 0) { return false; }
        if(l2 * c2 == 0) { return true; }
        return ((double)l1 * (double) c1) < ((double)l2 * (double)c2);
    };
    
    auto ic3 = [] (int c1, int l1, int c2, int l2) -> bool {
        return (c1 < c2);
    };
    
    auto rg1 = [] (int bc, int bl, int sbc, int sbl) -> double {
        return ((double)bl / (double)bc) - ((double)sbl / (double)sbc);
    };
    
    auto rg2 = [] (int bc, int bl, int sbc, int sbl) -> double {
        return abs((double)sbl * (double)sbc - (double)bl * (double)bc);
    };
    
    auto rc1 = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] < this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    auto rc2 = [this] (int r1, int r2) -> bool {
        return (this->g.cost[r1][r1 + this->g.g[graph_bundle].n] > this->g.cost[r2][r2 + this->g.g[graph_bundle].n]);
    };
    
    auto p = path();
    std::cout << "Heuristic solutions:         \t";
    
    
    auto h1 = max_regret_heuristic<decltype(ic1), decltype(rg1)>(g, ic1, rg1);
    p = h1.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }

    auto h2 = max_regret_heuristic<decltype(ic2), decltype(rg2)>(g, ic2, rg2);
    p = h2.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }
    
    auto h3 = heuristic_with_ordered_requests<decltype(rc1), decltype(ic3)>(g, rc1, ic3);
    p = h3.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }
    
    auto h4 = heuristic_with_ordered_requests<decltype(rc2), decltype(ic3)>(g, rc2, ic3);
    p = h4.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }
    
    auto h5 = best_insertion_heuristic<decltype(ic1)>(g, ic1);
    p = h5.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }
    
    auto h6 = best_insertion_heuristic<decltype(ic2)>(g, ic2);
    p = h6.solve();
    std::cout << p.total_cost << "\t";
    if(p.total_cost > 0) { paths.push_back(p); }
    
    std::cout << std::endl;
    
    auto appropriate_k_for_instance_size = 0u;
    
    for(const auto& limit_pair : params.ko.instance_size_limits) {
        if((unsigned int) g.g[graph_bundle].n <= limit_pair.n && limit_pair.k > appropriate_k_for_instance_size) {
            appropriate_k_for_instance_size = limit_pair.k;
        }
    }
    
    auto h7 = k_opt_heuristic(g, params, appropriate_k_for_instance_size, paths);
    auto k_opt_paths = h7.solve();
        
    std::cout << "Heuristic solutions (k = " << appropriate_k_for_instance_size << "): \t";
    for(const auto& path : k_opt_paths) {
        std::cout << path.total_cost << "\t";
    }
    std::cout << std::endl;
    
    paths.insert(paths.end(), k_opt_paths.begin(), k_opt_paths.end());
    
    return k_opt_paths;
}