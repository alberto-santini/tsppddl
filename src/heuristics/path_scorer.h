#ifndef PATH_SCORER
#define PATH_SCORER

#include <network/tsp_graph.h>
#include <network/path.h>

#include <limits>

struct path_scorer {
    virtual double operator()(const tsp_graph& g, const path& p) const = 0;
};

struct ps_cost : path_scorer {
    double operator()(const tsp_graph& g, const path& p) const {
        return p.total_cost;
    }
};

struct ps_cost_opposite : ps_cost {
    double operator()(const tsp_graph& g, const path& p) const {
        return -ps_cost::operator()(g, p);
    }
};

struct ps_cost_plus_load : path_scorer {
    double operator()(const tsp_graph& g, const path& p) const {
        return p.total_cost + p.total_load;
    }
};

struct ps_cost_plus_load_opposite : ps_cost_plus_load {
    double operator()(const tsp_graph& g, const path& p) const {
        return -ps_cost_plus_load::operator()(g, p);
    }
};

struct ps_load_times_cost : path_scorer {
    double operator()(const tsp_graph& g, const path& p) const {
        return p.total_load * p.total_cost;
    }
};

struct ps_load_times_cost_opposite : ps_load_times_cost {
    double operator()(const tsp_graph& g, const path& p) const {
        return -ps_load_times_cost::operator()(g, p);
    }
};

struct ps_capacity_usage_with_draught : path_scorer {
    double operator()(const tsp_graph& g, const path& p) const {
        auto min3 = [] (double x, double y, double z) { return std::min(x, std::min(y, z)); };
        auto Q = g.g[graph_bundle].capacity;
        auto residual_capacity = 0;
        
        for(auto i = 0u; i < p.length() - 1; ++i) {
            auto load = p.load_v[i];
            auto capacity = min3(Q, g.draught[p.path_v[i]], g.draught[p.path_v[i+1]]);
            
            residual_capacity += (capacity - load);
        }
        
        return -residual_capacity;
    }
};

struct ps_cost_times_capacity_usage : ps_capacity_usage_with_draught {
    double operator()(const tsp_graph& g, const path& p) const {
        return -p.total_cost * ps_capacity_usage_with_draught::operator()(g, p);
    }
};

#endif