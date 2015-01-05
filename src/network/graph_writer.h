#ifndef GRAPH_WRITER_H
#define GRAPH_WRITER_H

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <network/tsp_graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <iostream>

class graph_writer {
    const tsp_graph& g;
    const std::vector<std::vector<double>> x;
    const std::vector<std::vector<double>> y;
    
    static constexpr double eps = 0.0001;
    
public:
    struct vertex_label_writer {
        const tsp_graph& g;
    
        vertex_label_writer(const tsp_graph& g) : g{g} {}
    
        template<class Vertex> void operator()(std::ostream& out, const Vertex& v) const;
    };

    struct edge_label_writer {
        const tsp_graph& g;
        const std::vector<std::vector<double>>& x;
        const std::vector<std::vector<double>>& y;
        double eps;
    
        edge_label_writer(const tsp_graph& g, const std::vector<std::vector<double>>& x, const std::vector<std::vector<double>>& y, double eps) : g{g}, x{x}, y{y}, eps{eps} {}
    
        template<class Edge> void operator()(std::ostream& out, const Edge& e) const;
    };

    struct graph_property_writer {
        void operator()(std::ostream& out);
    };

    struct vertex_index_map {
        const tsp_graph& g;
    
        typedef tsp_graph::vertex_t key_type;
        typedef int value_type;
        typedef boost::readable_property_map_tag category;
    
        vertex_index_map(const tsp_graph& g) : g{g} {}
    };

    struct arc_index_map {
        const tsp_graph& g;
    
        typedef tsp_graph::edge_t key_type;
        typedef int value_type;
        typedef boost::readable_property_map_tag category;
    
        arc_index_map(const tsp_graph& g) : g{g} {}
    };
    
    graph_writer(const tsp_graph& g, std::vector<std::vector<double>> x, std::vector<std::vector<double>> y) : g{g}, x{x}, y{y} {}
    void write(const std::string& where) const;
};

inline int get(const graph_writer::vertex_index_map& i, const tsp_graph::vertex_t& v) {
    return i.g.g[v].id;
}

inline int get(const graph_writer::arc_index_map& i, const tsp_graph::edge_t& e) {
    return i.g.g[e].id;
}

#endif