#ifndef GRAPH_WRITER_H
#define GRAPH_WRITER_H

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <network/graph.h>
#include <solver/bc/callbacks/callbacks_helper.h>

#include <iostream>

struct VertexLabelWriter {
    const Graph& g;
    
    VertexLabelWriter(const Graph& g) : g{g} {}
    
    template<class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
        auto n = g.g[graph_bundle].n;
        auto i = (int)g.g[v].id;
        
        out << "[" << std::endl;
        if(i == 0) {
            out << "\tlabel=\"start\"" << std::endl;
            out << "\tpos=\"0,5\"" << std::endl;
        }
        if(i == 2*n+1) {
            out << "\tlabel=\"end\"" << std::endl;
            out << "\tpos=\"" << 3 * (i-n) << ",5\"" << std::endl;
        }
        if(1 <= i && i <= n) {
            out << "\tlabel=\"" << i << ", d: " << g.g[v].demand << ", l: " << g.g[v].draught <<  "\"" << std::endl;
            out << "\tpos=\"" << 3 * i << ",1\"" << std::endl;
            out << "\tfontcolor=\"red\"" << std::endl;
        }
        if(n+1 <= i && i <= 2*n) {
            out << "\tlabel=\"" << (i-n) << ", d: " << g.g[v].demand << ", l: " << g.g[v].draught <<  "\"" << std::endl;
            out << "\tpos=\"" << 3 * (i-n) << ",9\"" << std::endl;
            out << "\tfontcolor=\"blue\"" << std::endl;
        }
        out << "]";
    }
};

struct EdgeLabelWriter {
    const Graph& g;
    const std::vector<std::vector<double>>& x;
    const std::vector<std::vector<double>>& y;
    double eps;
    
    EdgeLabelWriter(const Graph& g, const std::vector<std::vector<double>>& x, const std::vector<std::vector<double>>& y, double eps) : g{g}, x{x}, y{y}, eps{eps} {}
    
    template<class Edge>
    void operator()(std::ostream& out, const Edge& e) const {
        auto vertex_i = source(e, g.g);
        auto vertex_j = target(e, g.g);
        auto i = g.g[vertex_i].id;
        auto j = g.g[vertex_j].id;
        
        if(x.at(i).at(j) > eps) {
            out << "[label=\"x: " << x.at(i).at(j) << ", y: " << y.at(i).at(j) << "\"]";
        } else {
            out << "[style=\"invis\"]";
        }
    }
};

struct GraphPropertyWriter {
    void operator()(std::ostream& out) {
        out << "graph[" << std::endl;
        out << "\tsplines=true" << std::endl;
        out << "\tesep=1" << std::endl;
        out << "];" << std::endl;
    }
};

struct VertexIndexMap {
    const Graph& g;
    
    typedef vertex_t key_type;
    typedef int value_type;
    typedef boost::readable_property_map_tag category;
    
    VertexIndexMap(const Graph& g) : g{g} {}
};

inline int get(const VertexIndexMap& i, const vertex_t& v) {
    return i.g.g[v].id;
}

struct ArcIndexMap {
    const Graph& g;
    
    typedef edge_t key_type;
    typedef int value_type;
    typedef boost::readable_property_map_tag category;
    
    ArcIndexMap(const Graph& g) : g{g} {}
};

inline int get(const ArcIndexMap& i, const edge_t& e) {
    return i.g.g[e].id;
}

class GraphWriter {
    const Graph& g;
    const std::vector<std::vector<double>> x;
    const std::vector<std::vector<double>> y;
    
public:
    GraphWriter(const Graph& g, std::vector<std::vector<double>> x, std::vector<std::vector<double>> y) : g{g}, x{x}, y{y} {}
    void write(const std::string& where) const;
};

#endif