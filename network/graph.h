#ifndef GRAPH_H
#define GRAPH_H

#include <network/node.h>
#include <network/arc.h>
#include <network/graph_info.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <vector>

using namespace boost;

typedef std::vector<int> demand_t;
typedef std::vector<int> draught_t;
typedef int cost_val_t;
typedef std::vector<cost_val_t> cost_row_t;
typedef std::vector<cost_row_t> cost_t;

typedef adjacency_list<listS, listS, directedS, Node, Arc, GraphInfo> graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef graph_traits<graph_t>::edge_descriptor edge_t;
typedef graph_traits<graph_t>::vertex_iterator vi_t;
typedef graph_traits<graph_t>::edge_iterator ei_t;

class Graph {    
public:
    graph_t g;
    demand_t demand;
    draught_t draught;
    cost_t cost;
    
    Graph(const demand_t demand, const draught_t draught, const cost_t cost, const int capacity);
    Graph make_reverse_graph() const;
};

#endif