#ifndef GRAPH_H
#define GRAPH_H

#include <network/node.h>
#include <network/arc.h>
#include <network/graph_info.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <vector>

using namespace boost;

using demand_t = std::vector<int>;
using draught_t = std::vector<int>;
using cost_val_t = int;
using cost_row_t = std::vector<cost_val_t>;
using cost_t = std::vector<cost_row_t>;

using graph_t = adjacency_list<listS, listS, directedS, Node, Arc, GraphInfo>;
using vertex_t = graph_traits<graph_t>::vertex_descriptor;
using edge_t = graph_traits<graph_t>::edge_descriptor;
using vi_t = graph_traits<graph_t>::vertex_iterator;
using ei_t = graph_traits<graph_t>::edge_iterator;

class Graph {    
public:
    graph_t g;
    demand_t demand;
    draught_t draught;
    cost_t cost;
    
    Graph(const demand_t& demand, const draught_t& draught, const cost_t& cost, int capacity);
    Graph make_reverse_graph() const;
};

#endif