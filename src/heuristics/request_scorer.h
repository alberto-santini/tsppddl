#ifndef REQUEST_SCORER
#define REQUEST_SCORER

#include <network/tsp_graph.h>

struct request_scorer {
    virtual double operator()(const tsp_graph& g, int request) const = 0;
};

struct rs_origin_destination_distance : request_scorer {
    double operator()(const tsp_graph& g, int request) const {
        int n = g.g[graph_bundle].n;
        
        assert(1 <= request && request <= n);
        
        return g.cost[request][request + n];
    }
};

struct rs_origin_destination_distance_opposite : rs_origin_destination_distance {
    double operator()(const tsp_graph& g, int request) const {
        return -rs_origin_destination_distance::operator()(g, request);
    }
};

struct rs_draught_demand_difference : request_scorer {
    double operator()(const tsp_graph& g, int request) const {
        auto n = g.g[graph_bundle].n;
        
        assert(1 <= request && request <= n);
        
        return (std::min(g.draught[request], g.draught[request + n]) - g.demand[request]);
    }
};

struct rs_draught_demand_difference_opposite : rs_draught_demand_difference {
    double operator()(const tsp_graph& g, int request) const {
        return -rs_draught_demand_difference::operator()(g, request);
    }
};

#endif