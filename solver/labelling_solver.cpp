#ifndef LABELLING_SOLVER_CPP
#define LABELLING_SOLVER_CPP

#include <iostream>
#include <list>
#include <memory>
#include <algorithm>

#include <boost/graph/r_c_shortest_paths.hpp>

#include <solver/labelling_solver.h>

std::vector<std::pair<Path, double>> LabellingSolver::get_undominated_paths() const {
    std::vector<Path> undominated_paths;
    std::vector<Label> undominated_labels;
    
    NodeIdFunctor nf(graph);
    ArcIdFunctor af(graph);
    
    boost::r_c_shortest_paths(
        graph->g,
        make_property_map<Vertex>(nf),
        make_property_map<Edge>(nf),
        graph->get_vertex(0),
        graph->get_vertex(2* graph->g[graph_bundle].num_requests + 1),
        undominated_paths,
        undominated_labels,
        Label(0, 0, 0, 0),
        LabelExtender(),
        LabelDominator(),
        std::allocator<boost::r_c_shortest_paths_label<BoostGraph, Label>>(),
        boost::default_r_c_shortest_paths_visitor());
        
    std::vector<std::pair<Path, double>> upaths_with_cost;
    for(Path p : undominated_paths) {
        upaths_with_cost.push_back(std::make_pair(p, graph->cost(p)));
    }
    
    return upaths_with_cost;
}

void LabellingSolver::solve() const {
    std::vector<std::pair<Path, double>> upaths_with_cost = get_undominated_paths();
    
    std::cout << "Labelling produced " << upaths_with_cost.size() << " undominated paths" << std::endl;
    for(std::pair<Path, double>& pc : upaths_with_cost) {
        std::reverse(pc.first.begin(), pc.first.end());
        
        std::cout << "Path with cost " << pc.second << std::endl;
        graph->print_path(pc.first);
    }
}

#endif