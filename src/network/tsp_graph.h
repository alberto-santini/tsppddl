#ifndef TSP_GRAPH_H
#define TSP_GRAPH_H

#include <network/node.h>
#include <network/arc.h>
#include <network/graph_info.h>

#include <boost/functional/hash.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cxxabi.h>

using namespace boost;

struct tsp_graph {
    using demand_t = std::vector<int>;
    using draught_t = std::vector<int>;
    using cost_val_t = int;
    using cost_row_t = std::vector<cost_val_t>;
    using cost_t = std::vector<cost_row_t>;

    using graph_t = adjacency_list<listS, listS, directedS, node, arc, graph_info>;
    using vertex_t = graph_traits<graph_t>::vertex_descriptor;
    using edge_t = graph_traits<graph_t>::edge_descriptor;
    using vi_t = graph_traits<graph_t>::vertex_iterator;
    using ei_t = graph_traits<graph_t>::edge_iterator;

    using infeasible_paths_map = std::unordered_map<std::vector<int>, bool, boost::hash<std::vector<int>>>;
    
    graph_t g;
    demand_t demand;
    draught_t draught;
    cost_t cost;
    
    infeasible_paths_map infeas_list;

    tsp_graph() {}
    tsp_graph(const demand_t& demand, const draught_t& draught, const cost_t& cost, int capacity, std::string instance_path);
    
    tsp_graph make_reverse_tsp_graph() const;
    bool is_path_eliminable(int i, int j, int k) const;
    void populate_list_of_infeasible_3_paths();
};

inline std::string classname(const std::type_info& ti) {
    int status;
    return abi::__cxa_demangle(ti.name(), 0, 0, &status);
}

#endif