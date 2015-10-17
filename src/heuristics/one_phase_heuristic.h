#ifndef ONE_PHASE_HEURISTIC
#define ONE_PHASE_HEURISTIC

#include <network/tsp_graph.h>
#include <network/path.h>

#include <vector>
#include <algorithm>
#include <limits>

#include <boost/optional.hpp>

template<class I>
struct one_phase_heuristic {
    const tsp_graph& g;
    const I& inserter;
    
    one_phase_heuristic(const tsp_graph& g, const I& inserter) : g(g), inserter(inserter) {
        std::cout << "one_phase_heuristic\t" << classname(typeid(this)) << "\t" << this << std::endl;
        std::cout << ">>> inserter\t" << classname(typeid(&inserter)) << "\t" << &inserter << std::endl << std::endl;
    }
    boost::optional<path> solve() const;
};

template<class I>
boost::optional<path> one_phase_heuristic<I>::solve() const {
    path p; // Path to be built
    int n = this->g.g[graph_bundle].n; // Number of requests
    std::vector<int> R(n); // Requests to be inserted
    
    p.path_v.reserve(2 * n + 2); p.load_v.reserve(2 * n + 2);
    p.path_v.push_back(0); p.path_v.push_back(2*n+1);
    p.load_v.push_back(0); p.load_v.push_back(0);
    
    // Fill the requests vector
    std::iota(R.begin(), R.end(), 1);
    
    while(!R.empty()) {
        // Score of the best insertion of the best request
        double best_score = std::numeric_limits<double>::min();
        // Best new path (the one where the best insertion of the best request is performed, starting from p)
        path best_path;
        // Insertion corresponding to the best_path
        int best_insertion;
        
        for(auto i : R) {
            bool success;
            double new_score;
            path new_path;
            
            std::tie(success, new_score, new_path) = this->inserter(this->g, p, i);
            
            if(success && new_score > best_score) {
                best_score = new_score;
                best_path = new_path;
                best_insertion = i;
            }
        }
        
        if(best_score == std::numeric_limits<double>::min()) {
            return boost::none;
        }
        
        // Update the current path p
        p = best_path;
        
        // Remove best_insertion from R
        R.erase(std::remove(R.begin(), R.end(), best_insertion), R.end());
    }
    
    return p;
}

#endif