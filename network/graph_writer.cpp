#include <network/graph_writer.h>

#include <boost/graph/graphviz.hpp>

#include <fstream>

void GraphWriter::write(const std::string& where) const {
    std::ofstream gv_file;
    gv_file.open(where + ".dot", std::ios::out);
    
    write_graphviz(gv_file, g.g, VertexLabelWriter(g), EdgeLabelWriter(g, x, y, 0.00001), GraphPropertyWriter(), VertexIndexMap(g));
}