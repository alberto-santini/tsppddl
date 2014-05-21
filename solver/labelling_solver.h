#ifndef LABELLING_SOLVER_H
#define LABELLING_SOLVER_H

#include <memory>
#include <utility>
#include <vector>

#include <network/graph.h>
#include <parser/data.h>
#include <solver/label.h>

class LabellingSolver {
    std::shared_ptr<Graph> graph;
    
    std::vector<std::pair<Path, double>> get_undominated_paths() const;
    
public:
    LabellingSolver(std::shared_ptr<Graph> graph) : graph(graph) {}
    void solve() const;
};

#endif