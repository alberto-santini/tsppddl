#include <solver/heuristics/heuristic_solver.h>

#include <heuristics/best_insertion_heuristic.h>
#include <heuristics/heuristic_with_ordered_requests.h>
#include <heuristics/k_opt_heuristic.h>
#include <heuristics/max_regret_heuristic.h>

#include <iostream>

std::vector<Path> HeuristicSolver::solve() const {
    auto paths = std::vector<Path>();
        
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
    
    auto p = Path();
    
    auto h1 = MaxRegretHeuristic<decltype(ic1), decltype(rg1)>(g, ic1, rg1);
    p = h1.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);

    auto h2 = MaxRegretHeuristic<decltype(ic2), decltype(rg2)>(g, ic2, rg2);
    p = h2.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);
    
    auto h3 = HeuristicWithOrderedRequests<decltype(rc1), decltype(ic3)>(g, rc1, ic3);
    p = h3.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);
    
    auto h4 = HeuristicWithOrderedRequests<decltype(rc2), decltype(ic3)>(g, rc2, ic3);
    p = h4.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);
    
    auto h5 = BestInsertionHeuristic<decltype(ic1)>(g, ic1);
    p = h5.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);
    
    auto h6 = BestInsertionHeuristic<decltype(ic2)>(g, ic2);
    p = h6.solve();
    std::cout << "Heuristic solution: " << p.total_cost << std::endl;
    paths.push_back(p);
    
    for(auto k = 2; k < 5; k++) {
        auto h7 = KOptHeuristic(g, params, k, paths);
        p = h7.solve();
        std::cout << "Heuristic solution (k = " << k << "): " << p.total_cost << std::endl;
        paths.push_back(p);
    }
    
    return paths;
}