#ifndef TABU_SOLVER_H
#define TABU_SOLVER_H

#include <network/tsp_graph.h>
#include <network/path.h>
#include <program/program_data.h>
#include <parser/program_params.h>

#include <utility>
#include <vector>

class tabu_solver {
public:
    tabu_solver(tsp_graph& g, const program_params& params, program_data& data, std::vector<path> initial_solutions);
    std::vector<path> solve();
    std::vector<path> solve_sequential();
    void solve_parameter_tuning();

    struct tabu_move {
        std::pair<int,int> vertices;
        int edge_cost;

        tabu_move() : vertices{std::make_pair(-1,-1)}, edge_cost{-1} {}
        tabu_move(std::pair<int, int> vertices, int edge_cost) : vertices{vertices}, edge_cost{edge_cost} {}
        bool operator==(const tabu_move& other) const {
            return (
                (vertices.first == other.vertices.first && vertices.second == other.vertices.second) ||
                (vertices.first == other.vertices.second && vertices.second == other.vertices.first)
            );
        }
    };

    struct tabu_result {
        path p;
        tabu_move shortest_erased_edge;

        tabu_result(path p, tabu_move shortest_erased_edge) : p{p}, shortest_erased_edge{shortest_erased_edge} {}
        inline bool empty() { return p.path_v.empty(); }
    };

    struct tabu_and_non_tabu_solutions {
        tabu_result overall_best;
        tabu_result best_without_tabu;

        tabu_and_non_tabu_solutions(tabu_result overall_best, tabu_result best_without_tabu) : overall_best{overall_best}, best_without_tabu{best_without_tabu} {}
    };
    
private:    
    tsp_graph&              g;
    const program_params&   params;
    program_data&           data;
    std::vector<path>       initial_solutions;
    std::vector<path>       sliced_initial_solutions;
    int                     tabu_list_size;
    
    static constexpr double eps = 0.0001;

    path tabu_search(path init_sol);
    void update_tabu_list(std::vector<tabu_move>& tabu_list, const tabu_solver::tabu_result& sol);
    void print_results(const std::vector<path>& solutions) const;
};

#endif