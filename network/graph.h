#ifndef GRAPH_H
#define GRAPH_H

#include <memory>
#include <string>
#include <iostream>

#include <network/node.h>
#include <network/arc.h>
#include <network/graph_properties.h>
#include <parser/parser.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/property_map.hpp>

using namespace boost;

typedef adjacency_list<listS, listS, bidirectionalS, std::shared_ptr<Node>, std::shared_ptr<Arc>, GraphProperties> BoostGraph;

typedef graph_traits<BoostGraph>::vertex_iterator vit;
typedef graph_traits<BoostGraph>::edge_iterator eit;
typedef graph_traits<BoostGraph>::in_edge_iterator ieit;
typedef graph_traits<BoostGraph>::out_edge_iterator oeit;

typedef graph_traits<BoostGraph>::vertex_descriptor Vertex;
typedef graph_traits<BoostGraph>::edge_descriptor Edge;

typedef std::vector<Edge> Path;

class Graph {
    void generate_distance_table();
    Path order_path(Path up) const;
    
public:
    BoostGraph g;
    std::string name;
    std::vector<std::vector<double>> handy_dt;
    
    Graph(std::shared_ptr<Data> data);
    void print_data() const;
    void print_graphviz(const std::string file_name) const;
    void print_path(Path p) const;
    void print_unordered_path(Path up) const;
    Vertex get_vertex(const int id) const;
    Edge get_edge(const int source_id, const int target_id) const;
    double cost(const Path& p) const;
};

// Stuff needed to output the graph as Graphviz:

class VertexLabelWriter {
    const Graph* graph;
public:
    VertexLabelWriter(const Graph* graph) : graph(graph) {}
    void operator()(std::ostream& out, const Vertex& v) const {
        std::shared_ptr<Node> node = graph->g[v];
        if(node->id == 0) {
            out << "[label=\"0\"]";
        } else if(node->id == 2 * graph->g[graph_bundle].num_requests + 1) {
            out << "[label=\"2n+1\"]";
        } else if(node->id <= graph->g[graph_bundle].num_requests) {
            out << "[label=\"o(" << node->id << ")\"]";
        } else if(node->id > graph->g[graph_bundle].num_requests) {
            out << "[label=\"d(" << (node->id - graph->g[graph_bundle].num_requests) << ")\"]";
        } else {
            out << "[label=\"?\"]";
        }
    }
};

class EdgeLabelWriter {
    const Graph* graph;
public:
    EdgeLabelWriter(const Graph* graph) : graph(graph) {}
    void operator()(std::ostream& out, const Edge& e) const {
        std::shared_ptr<Arc> arc = graph->g[e];
        out << "[label=\"" << arc->cost << "\", color=\"blue\"]";
    }
};

class GraphLabelWriter {
public:
    void operator()(std::ostream& out) const {}
};

class VertexIndexMap {
public:
    const Graph* graph;
    
    typedef Vertex key_type;
    typedef int value_type;
    typedef boost::readable_property_map_tag category;
    
    VertexIndexMap(const Graph* graph) : graph(graph) {}
};

inline int get(const VertexIndexMap& i, const Vertex& v) {
    return i.graph->g[v]->id;
}

class ArcIndexMap {
public:
    const Graph* graph;
    
    typedef Edge key_type;
    typedef int value_type;
    typedef boost::readable_property_map_tag category;
    
    ArcIndexMap(const Graph* graph) : graph(graph) {}
};

inline int get(const ArcIndexMap& i, const Edge& e) {
    return i.graph->g[e]->id;
}

#endif