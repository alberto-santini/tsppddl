#ifndef TWO_PHASE_HEURISTIC
#define TWO_PHASE_HEURISTIC

#include <network/tsp_graph.h>
#include <network/path.h>
#include <heuristics/inserter.h>

#include <vector>
#include <algorithm>
#include <limits>

#include <boost/optional.hpp>

template<class RS, class IS>
struct two_phase_heuristic {
    const tsp_graph& g;
    const RS& r_scorer;
    const IS& ins_scorer;
    
    struct scored_request { int i; double score; };
    struct scored_request_comparator { bool operator()(const scored_request& lhs, const scored_request& rhs) const { return lhs.score > rhs.score; /* lhs goes *before* rhs iff its score is higher */ } };
    
    two_phase_heuristic(const tsp_graph& g, const RS& r_scorer, const IS& ins_scorer) : g(g), r_scorer(r_scorer), ins_scorer(ins_scorer) {}
    boost::optional<path> solve() const;
};

template<class RS, class IS>
boost::optional<path> two_phase_heuristic<RS, IS>::solve() const {
    path p; // Path to be built
    int n = this->g.g[graph_bundle].n; // Number of requests
    
    p.path_v.reserve(2 * n + 2); p.load_v.reserve(2 * n + 2);
    p.path_v.push_back(0); p.path_v.push_back(2*n+1);
    p.load_v.push_back(0); p.load_v.push_back(0);
    
    std::set<scored_request, scored_request_comparator> R;
    
    for(int i = 1; i <= n; ++i) {
        R.insert({i, this->r_scorer(g, i)});
    }
    
    normal_inserter<IS> i(ins_scorer);
    
    for(const auto& sr : R) {
        bool success;
        double score;
        path new_path;
        
        std::tie(success, score, new_path) = i(this->g, p, sr.i);
        
        if(!success) {
            return boost::none;
        }
        
        p = new_path;
    }
    
    return p;
}

#endif