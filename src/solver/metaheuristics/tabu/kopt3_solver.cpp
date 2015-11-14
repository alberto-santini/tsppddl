#include <solver/metaheuristics/tabu/kopt3_solver.h>

#include <boost/optional.hpp>

#include <stdexcept>

tabu_solver::tabu_and_non_tabu_solutions kopt3_solver::solve(const path& starting_solution, const std::vector<tabu_solver::tabu_move>& tabu_moves) {
    auto i = std::vector<int>(3);
    auto j = std::vector<int>(3);
    
    auto new_shortest_path_overall = path();
    auto new_shortest_path_halal = path(); // Halal = non-tabu
    auto new_tabu_move_overall = tabu_solver::tabu_move();
    auto new_tabu_move_halal = tabu_solver::tabu_move(); // Halal = non-tabu
    auto overall_sol_found = false;
    auto halal_sol_found = false;

    for(auto _i0 = 0u; _i0 < starting_solution.path_v.size() - 3; _i0++) {
        i[0] = starting_solution.path_v.at(_i0);
        j[0] = starting_solution.path_v.at(_i0 + 1);
        
        for(auto _i1 = _i0 + 1; _i1 < starting_solution.path_v.size() - 2; _i1++) {
            i[1] = starting_solution.path_v.at(_i1);
            j[1] = starting_solution.path_v.at(_i1 + 1);
            
            for(auto _i2 = _i1 + 1; _i2 < starting_solution.path_v.size() - 1; _i2++) {
                i[2] = starting_solution.path_v.at(_i2);
                j[2] = starting_solution.path_v.at(_i2 + 1);
                
                auto shortest_id = 0;
                
                if(g.cost[i[1]][j[1]] < g.cost[i[0]][j[0]]) {
                    shortest_id = 1;
                }
                
                if(g.cost[i[2]][j[2]] < g.cost[i[1]][j[1]]) {
                    shortest_id = 2;
                }
                
                auto is_tabu = false;

                for(const auto& move : tabu_moves) {
                    if(
                        (move.vertices.first == i[shortest_id] && move.vertices.second == j[shortest_id]) ||
                        (move.vertices.first == j[shortest_id] && move.vertices.second == i[shortest_id])
                    ) {
                        is_tabu = true;
                    }
                }

                auto new_path = exec_3opt(starting_solution, i, j);
                
                if(new_path) {
                    if(!overall_sol_found) {
                        overall_sol_found = true;
                        new_shortest_path_overall = *new_path;
                        new_tabu_move_overall = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);
                    } else {
                        if((*new_path).total_cost < new_shortest_path_overall.total_cost) {
                            new_shortest_path_overall = *new_path;
                            new_tabu_move_overall = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);
                        }
                    }

                    if(!is_tabu) {
                        if(!halal_sol_found) {
                            halal_sol_found = true;
                            new_shortest_path_halal = *new_path;
                            new_tabu_move_halal = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);
                        } else {
                            if((*new_path).total_cost < new_shortest_path_halal.total_cost) {
                                new_shortest_path_halal = *new_path;
                                new_tabu_move_halal = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);
                            }
                        }
                    }
                }
            }
        }
    }
    
    return tabu_solver::tabu_and_non_tabu_solutions(
            std::move(tabu_solver::tabu_result(std::move(new_shortest_path_overall), std::move(new_tabu_move_overall))),
            std::move(tabu_solver::tabu_result(std::move(new_shortest_path_halal), std::move(new_tabu_move_halal)))
    );
}

boost::optional<path> kopt3_solver::exec_3opt(const path& p, const std::vector<int>& i, const std::vector<int>& j) {
    auto n = g.g[graph_bundle].n;
    auto p_x = p.get_x_values(n);
    
    for(auto n = 0; n <= 2; n++) {
        p_x[i[n]][j[n]] = 0;
    }
    
    if(j[1] > n && ((j[0] <= n && j[0] + n == j[1]) || (i[1] <= n && i[1] + n == j[1]))) {
        return boost::none;
    }
    
    if(i[2] > n && ((j[0] <= n && j[0] + n == i[2]) || (i[1] <= n && i[1] + n == i[2]))) {
        return boost::none;
    }
    
    p_x[i[0]][j[1]] = 1;
    p_x[i[1]][j[2]] = 1;
    p_x[i[2]][j[0]] = 1;
    
    auto new_path = path(g, p_x);
    
    if(!new_path.verify_feasible(g)) {
        return boost::none;
    }
    
    return new_path;
}