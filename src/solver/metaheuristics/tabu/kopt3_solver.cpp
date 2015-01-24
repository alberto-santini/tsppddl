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

    if(DEBUG) {
        std::cerr << "******************** K-OPT ********************" << std::endl;
    }

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

                if(DEBUG) {
                    std::cerr << "Starting path: ";
                    starting_solution.print(std::cerr);
                    std::cerr << std::endl;
                    std::cerr << "Checking move (" << i[0] << "," << j[0] << ")-(" << i[1] << "," << j[1] << ")-(" << i[2] << "," << j[2] << ") against " << tabu_moves.size() << " tabu moves" << std::endl;
                }

                for(const auto& move : tabu_moves) {
                    if(
                        (move.vertices.first == i[shortest_id] && move.vertices.second == j[shortest_id]) ||
                        (move.vertices.first == j[shortest_id] && move.vertices.second == i[shortest_id])
                    ) {
                        is_tabu = true;
                    }
                }

                if(DEBUG) {
                    std::cerr << "\tIs tabu? " << std::boolalpha << is_tabu << std::endl;
                }

                auto new_path = exec_3opt(starting_solution, i, j);
                
                if(new_path) {
                    if(DEBUG) {
                        std::cerr << "Move gives a feasible new path" << std::endl;
                    }

                    if(!overall_sol_found) {
                        if(DEBUG) {
                            std::cerr << "\tFirst overall solution found: I take it no matter the cost (" << (*new_path).total_cost << ")" << std::endl;
                        }

                        overall_sol_found = true;
                        new_shortest_path_overall = *new_path;
                        new_tabu_move_overall = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);

                        if(DEBUG) {
                            std::cerr << "\tNew overall shortest path: ";
                            new_shortest_path_overall.print(std::cerr);
                            std::cerr << std::endl;
                            std::cerr << "\tNew overall tabu move: (" << new_tabu_move_overall.vertices.first << "," << new_tabu_move_overall.vertices.second << ")" << std::endl;
                        }
                    } else {
                        if((*new_path).total_cost < new_shortest_path_overall.total_cost) {
                            if(DEBUG) {
                                std::cerr << "\tThis new solution improves over the current overall best (" << (*new_path).total_cost << " < " << new_shortest_path_overall.total_cost << ")" << std::endl;
                            }

                            new_shortest_path_overall = *new_path;
                            new_tabu_move_overall = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);

                            if(DEBUG) {
                                std::cerr << "\tNew overall shortest path: ";
                                new_shortest_path_overall.print(std::cerr);
                                std::cerr << std::endl;
                                std::cerr << "\tNew overall tabu move: (" << new_tabu_move_overall.vertices.first << "," << new_tabu_move_overall.vertices.second << ")" << std::endl;
                            }
                        } else if(DEBUG) {
                            std::cerr << "\tUnfortunately this new solution does not improve over the overall best" << std::endl;
                        }
                    }

                    if(!is_tabu) {
                        if(DEBUG) {
                            std::cerr << "The new solution happens to be halal!" << std::endl;
                        }

                        if(!halal_sol_found) {
                            if(DEBUG) {
                                std::cerr << "\tFirst halal solution found: I take it no matter the cost (" << (*new_path).total_cost << ")" << std::endl;
                            }

                            halal_sol_found = true;
                            new_shortest_path_halal = *new_path;
                            new_tabu_move_halal = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);

                            if(DEBUG) {
                                std::cerr << "\tNew halal shortest path: ";
                                new_shortest_path_halal.print(std::cerr);
                                std::cerr << std::endl;
                                std::cerr << "\tNew halal tabu move: (" << new_tabu_move_halal.vertices.first << "," << new_tabu_move_halal.vertices.second << ")" << std::endl;
                            }
                        } else {
                            if((*new_path).total_cost < new_shortest_path_halal.total_cost) {
                                if(DEBUG) {
                                    std::cerr << "\tThis new solution improves over the current halal best (" << (*new_path).total_cost << " < " << new_shortest_path_halal.total_cost << ")" << std::endl;
                                }

                                new_shortest_path_halal = *new_path;
                                new_tabu_move_halal = tabu_solver::tabu_move(std::make_pair(i[shortest_id], j[shortest_id]), g.cost[i[shortest_id]][j[shortest_id]]);

                                if(DEBUG) {
                                    std::cerr << "\tNew halal shortest path: ";
                                    new_shortest_path_halal.print(std::cerr);
                                    std::cerr << std::endl;
                                    std::cerr << "\tNew halal tabu move: (" << new_tabu_move_halal.vertices.first << "," << new_tabu_move_halal.vertices.second << ")" << std::endl;
                                }
                            } else if(DEBUG) {
                                std::cerr << "\tUnfortunately this new solution does not improve over the halal best" << std::endl;
                            }
                        }
                    }
                } else if(DEBUG) {
                    std::cerr << "New path not feasible" << std::endl;
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
    auto p_x = p.get_x_values(g.g[graph_bundle].n);
    
    for(auto n = 0u; n <= 2u; n++) {
        p_x[i[n]][j[n]] = 0;
    }
    
    p_x[i[0]][j[1]] = 1;
    p_x[i[1]][j[2]] = 1;
    p_x[i[2]][j[0]] = 1;
    
    auto new_path = path(g, p_x);
    
    try {
        new_path.verify_feasible(g);
    } catch(std::runtime_error& e) {
        return boost::none;
    }
    
    return new_path;
}