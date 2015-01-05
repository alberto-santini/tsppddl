#include <network/graph_writer.h>

#include <boost/graph/graphviz.hpp>

#include <fstream>

void graph_writer::write(const std::string& where) const {
    std::ofstream gv_file;
    gv_file.open(where + ".dot", std::ios::out);
    
    write_graphviz(gv_file, g.g, vertex_label_writer(g), edge_label_writer(g, x, y, eps), graph_property_writer(), vertex_index_map(g));
}

template<class Vertex>
void graph_writer::vertex_label_writer::operator()(std::ostream& out, const Vertex& v) const {
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

template<class Edge>
void graph_writer::edge_label_writer::operator()(std::ostream& out, const Edge& e) const {
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

void graph_writer::graph_property_writer::operator()(std::ostream& out) {
    out << "graph[" << std::endl;
    out << "\tsplines=true" << std::endl;
    out << "\tesep=1" << std::endl;
    out << "];" << std::endl;
}