#ifndef PATH_SCORER
#define PATH_SCORER

#include <network/tsp_graph.h>
#include <network/path.h>

#include <limits>

struct path_scorer {
    path_scorer() {
        std::cout << "path_scorer\t\t" << classname(typeid(this)) << "\t\t" << this << std::endl << std::endl;
    }
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

struct ps_load_over_cost : path_scorer {
    double operator()(const tsp_graph& g, const path& p) const {
        if(p.total_cost == 0) { return std::numeric_limits<double>::max(); }
        return p.total_load / p.total_cost;
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

#endif