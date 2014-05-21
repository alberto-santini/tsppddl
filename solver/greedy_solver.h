#ifndef GREEDY_SOLVER_H
#define GREEDY_SOLVER_H

#include <memory>
#include <utility>
#include <vector>

#include <network/graph.h>

class GreedySolver {
    int runs_number;
    int best_arcs;
    double visitable_coeff;
    std::shared_ptr<Graph> graph;
    
    std::vector<std::shared_ptr<Node>> updated_visitable_origins(const std::vector<std::shared_ptr<Node>> visited_ports, const int load) const;
    bool contains(const std::vector<std::shared_ptr<Node>> visited, const int id) const;
    Path generate_path() const;
    std::tuple<Path, double, int> get_best_path() const;
    
public:
    GreedySolver(const int runs_number, const int best_arcs, const double visitable_coeff, std::shared_ptr<Graph> graph) : runs_number(runs_number), best_arcs(best_arcs), visitable_coeff(visitable_coeff), graph(graph) {}
    void solve() const;
};

#endif