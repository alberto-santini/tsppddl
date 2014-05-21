#ifndef HEURISTIC_SOLVER_CPP
#define HEURISTIC_SOLVER_CPP

#include <algorithm>
#include <limits>
#include <iostream>

#include <solver/heuristic_solver.h>

HeuristicSolver::HeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph) : data(data), graph(graph) {
    ship_capacity = data->capacity;
    n = data->num_requests;
    distance = std::vector<std::vector<double>>(2 * n + 2, std::vector<double>(2 * n + 2));
    draught = std::vector<int>(2 * n + 2);
    demand = std::vector<int>(2 * n + 2);
    
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(graph->g); vi != vi_end; ++vi) {
        std::shared_ptr<Node> n = graph->g[*vi];
        draught[n->id] = n->port->draught;
        demand[n->id] = n->demand;
        
        vit vi2, vi2_end;
        for(std::tie(vi2, vi2_end) = vertices(graph->g); vi2 != vi2_end; ++vi2) {
            std::shared_ptr<Node> n2 = graph->g[*vi2];
            distance[n->id][n2->id] = data->distances[n->port->id][n2->port->id];
        }
    }
}

std::tuple<std::vector<int>, std::vector<int>, int> HeuristicSolver::solve() const {
    typedef std::tuple<std::vector<int>, std::vector<int>, int, double, double> placement;
    
    std::vector<int> path; path.reserve(2 * n + 1);
    path.push_back(0); path.push_back(2 * n + 1);
    
    // std::cout << "Initialised path; size: " << path.size() << ", capacity: " << path.capacity() << std::endl;
    
    std::vector<int> partial_load; partial_load.reserve(2 * n + 1);
    partial_load.push_back(0); partial_load.push_back(0);
    
    // std::cout << "Initialised partial_load; size: " << partial_load.size() << ", capacity: " << partial_load.capacity() << std::endl;
    
    int load = 0;
    double length = 0;
    
    std::vector<int> remaining(n);
    for(int i = 0; i < n; i++) { remaining[i] = i + 1; }
    
    // std::cout << "Initialised remaining, size: " << remaining.size() << std::endl;
    
    while(remaining.size() != 0) {
        // std::cout << "*** Path: ";
        // for(int z = 0; z < path.size(); z++) { std::cout << path[z] << "(" << partial_load[z] << ") "; }
        // std::cout << "- length: " << length << std::endl;
        
        // Map:
        // request_id => placement
        // Placement:
        // (new_path, new_partial_load, new_load, new_length, new_metric)
        std::map<int, placement> best_placement;
        std::map<int, placement> second_best_placement;
        
        // std::cout << "Requests remaining: ";
        // for(int z = 0; z < remaining.size(); z++) { std::cout << remaining[z] << " "; }
        // std::cout << std::endl;
        
        for(int i : remaining) {
            // std::cout << "Considering to insert request " << i << " -> " << n+i << std::endl;
            
            
            double best_metric = 0;
            double second_best_metric = 0;
            bool insertable = false;
            
            for(int x = 1; x < path.size(); x++) {
                for(int y = x; y < path.size(); y++) {
                    std::vector<int> new_path(path.size() + 2);
                    std::vector<int> new_partial_load(partial_load.size() + 2);
                    
                    // std::cout << "\tInitialised new_path; size: " << new_path.size() << ", capacity: " << new_path.capacity() << std::endl;
                    // std::cout << "\tInitialised new_partial_load; size: " << new_partial_load.size() << ", capacity: " << new_partial_load.capacity() << std::endl;
                    
                    int new_load;
                    double new_length;
                    double new_metric;
                    bool feasible_and_improves;
                        
                    feasible_and_improves = new_path_if_feasible(x, y, i, length, load, second_best_metric, path, partial_load, new_path, new_partial_load, new_length, new_load, new_metric);
                        
                    if(feasible_and_improves) {
                        insertable = true;
                        // std::cout << "\t\tFound a feasible insertion that improves over ";
                        if(new_metric > best_metric) {
                            // std::cout << "the best metric" << std::endl;
                            second_best_placement[i] = best_placement[i];
                            second_best_metric = best_metric;
                            best_placement[i] = std::make_tuple(new_path, new_partial_load, new_load, new_length, new_metric);
                            best_metric = new_metric;
                        } else {
                            // std::cout << "the second best metric" << std::endl;
                            second_best_placement[i] = std::make_tuple(new_path, new_partial_load, new_load, new_length, new_metric);
                            second_best_metric = new_metric;
                        }
                    }
                }
            }
            
            if(!insertable) {
                // std::cout << "I couldn't find any feasible insertion for request " << i << std::endl;
                best_placement[i] = std::make_tuple(std::vector<int>(0), std::vector<int>(0), 0, std::numeric_limits<int>::max(), 0);
                second_best_placement[i] = std::make_tuple(std::vector<int>(0), std::vector<int>(0), 0, std::numeric_limits<int>::max(), 0);
            }
        }
        
        // std::cout << "Considered all the " << remaining.size() << " remaining requests" << std::endl;
        
        int req_with_greatest_diff = remaining[0];
        double greatest_diff = std::get<4>(best_placement[remaining[0]]) - std::get<4>(second_best_placement[remaining[0]]);
        placement pl = best_placement[remaining[0]];
        
        // std::cout << "Looking for a remaining request with metric better than those of remaning[0] (request " << remaining[0];
        // std::cout << ") with metric difference " << greatest_diff << std::endl;
        for(auto rem_it = remaining.begin() + 1; rem_it != remaining.end(); ++rem_it) {
            double diff = std::get<4>(best_placement[*rem_it]) - std::get<4>(second_best_placement[*rem_it]);
            
            // std::cout << "\tThe difference between best and second best metric for request " << *rem_it;
            // std::cout << " is: " << diff << std::endl;
            
            if(diff > greatest_diff) {
                req_with_greatest_diff = *rem_it;
                greatest_diff = diff;
                pl = best_placement[*rem_it];
            }
        }
        
        if(greatest_diff == 0 && std::get<4>(pl) == 0) {
            // std::cout << "No feasible request to insert!" << std::endl;
            return std::make_tuple(std::vector<int>(0), std::vector<int>(0), -1);
        }
        
        // std::cout << "The request with the greatest difference is " << req_with_greatest_diff << " with a difference of ";
        // std::cout << greatest_diff << std::endl;
        
        path = std::get<0>(pl);
        partial_load = std::get<1>(pl);
        load = std::get<2>(pl);
        length = std::get<3>(pl);
        
        // std::cout << "Size of remaining: " << remaining.size() << std::endl;
        // std::cout << "Remaining: ";
        // for(int z = 0; z < remaining.size(); z++) { std::cout << remaining[z] << " "; }
        // std::cout << std::endl;
        
        remaining.erase(std::remove(remaining.begin(), remaining.end(), req_with_greatest_diff), remaining.end());
        
        // std::cout << "Removed " << req_with_greatest_diff << " from remaining; new size: " << remaining.size() << std::endl;
        // std::cout << "Remaining is now: ";
        // for(int z = 0; z < remaining.size(); z++) { std::cout << remaining[z] << " "; }
        // std::cout << std::endl;
        
        // std::cout << "New partial path length: " << length << "; should be: " << test_path_cost(path) << "\tPath: ";
        // for(int z = 0; z < path.size(); z++) { std::cout << path[z] << " "; }
        // std::cout << std::endl;
    }
    
    std::cout << "PATH GENERATED: ";
    for(int z = 0; z < path.size(); z++) { std::cout << path[z] << " "; }
    std::cout << std::endl << "COST: " << length << std::endl;
    
    double tested_length = test_path_cost(path);
    std::cout << "RECOMPUTED COST: " << tested_length << std::endl;
    
    return std::make_tuple(path, partial_load, length);
}

double HeuristicSolver::test_path_cost(const std::vector<int>& path) const {
    double cost = 0; double load = 0;
    for(int i = 0; i < path.size() - 1; i++) {
        eit ei, ei_end;
        for(std::tie(ei, ei_end) = edges(graph->g); ei != ei_end; ++ei) {
            std::shared_ptr<Node> source_n = graph->g[source(*ei, graph->g)];
            std::shared_ptr<Node> target_n = graph->g[target(*ei, graph->g)];
            if(source_n->id == path[i] && target_n->id == path[i+1]) {
                cost += graph->g[*ei]->cost;
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated upon arriving at " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
                load += target_n->demand;
                if(load > ship_capacity) {
                    std::cout << "Ship capacity violated after servicing " << target_n->id << " (" << load << " vs. capacity " << ship_capacity << ")" << std::endl;
                }
                if(load > target_n->port->draught) {
                    std::cout << "Port draught violated after servicing " << target_n->id << " (" << load << " vs. limit " << target_n->port->draught << ")" << std::endl;
                }
            }
        }
    }
    
    return cost;
}

bool HeuristicSolver::new_path_if_feasible(const int& x, const int& y, const int& i, const double& length, const int& load, const double& second_best_metric, const std::vector<int>& path, const std::vector<int>& partial_load, std::vector<int>& new_path, std::vector<int>& new_partial_load, double& new_length, int& new_load, double& new_metric) const {
    // std::cout << "\tEntered new_path_if_feasible(x: " << x << ", y: " << y << ", i: " << i << ", n+i: " << n+i << ")" << std::endl;
    // std::cout << "\tpath[x-1] = " << path[x-1] << ", path[y-1] = " << path[y-1] << ", path[x] = " << path[x] << ", path[y] = " << path[y] << std::endl;
    // std::cout << "\tdistance[path[x-1]][path[x]] = " << distance[path[x-1]][path[x]] << ", distance[path[y-1]][path[y]] = " << distance[path[y-1]][path[y]] << std::endl;
    // std::cout << "\tdistance[path[x-1]][i] = " << distance[path[x-1]][i] << ", distance[i][path[x]] = " << distance[i][path[x]] << std::endl;
    // std::cout << "\tdistance[path[y-1]][n+i] = " << distance[path[y-1]][n+i] << ", distance[n+i][path[y]] = " << distance[n+i][path[y]] << std::endl;
    if(x != y) {
        new_length =    length -
                        distance[path[x-1]][path[x]] -
                        distance[path[y-1]][path[y]] +
                        distance[path[x-1]][i] +
                        distance[i][path[x]] +
                        distance[path[y-1]][n+i] +
                        distance[n+i][path[y]];
    } else {
        // Insert i and n+i consecutively
        new_length =    length -
                        distance[path[x-1]][path[x]] +
                        distance[path[x-1]][i] +
                        distance[i][n+i] +
                        distance[n+i][path[x]];
    }
    new_load = load + demand[i];
    new_metric = new_load / new_length;
    
    // std::cout << "\tOld path length: " << length << ", with new request: " << new_length << std::endl;
    // std::cout << "\tOld carried load: " << load << ", with new request: " << new_load << std::endl;
    // std::cout << "\tNew metric: " << new_metric << std::endl;
    
    if(new_metric <= second_best_metric) {
        // std::cout << "\t\tThis metric is worse than the second best (" << second_best_metric << "): we discard the insertion" << std::endl;
        // Can't improve
        return false;
    }
    
    // std::cout << "\t\tThis metric is better than the second best (" << second_best_metric << "): we check if the insertion is feasible" << std::endl;
    // std::cout << "\t\t\tOld path: ";
    // for(int z = 0; z < path.size(); z++) { std::cout << path[z] << "(" << partial_load[z] << ") "; }
    // std::cout << std::endl;
    // std::cout << "\t\t\tNew path: ";
    
    if(x != y) {    
        for(int j = 0; j <= x-1; j++) {
            new_path[j] = path[j];
            new_partial_load[j] = partial_load[j];
            // std::cout << new_path[j] << "(" << new_partial_load[j] << ") ";
        }
    
        new_path[x] = i;
        new_partial_load[x] = new_partial_load[x-1] + demand[i];
        // std::cout << new_path[x] << "(" << new_partial_load[x] << ") ";
    
        if(new_partial_load[x] > std::min(std::min(draught[i], draught[path[x]]), ship_capacity)) {
            // std::cout << std::endl << "\t\t\t\tInfeasible: new partial load at " << new_path[x] << " is " << new_partial_load[x] << " > min(";
            // std::cout << "draught[" << i << "]: " << draught[i] << ", draught[" << path[x] << "]: " << draught[path[x]];
            // std::cout << ", Q: " << ship_capacity << ")" << std::endl;
            return false;
        }
    
        for(int j = x + 1; j <= y; j++) {
            new_path[j] = path[j-1];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j]];
        
            // std::cout << new_path[j] << "(" << new_partial_load[j] << ") ";
        
            int next_port = (j < y ? path[j] : n+i);
            int next_port_draught = (j < y ? draught[path[j]] : draught[n+i]);
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                // std::cout << std::endl << "\t\t\t\tInfeasible: new partial load at " << new_path[j] << " is " << new_partial_load[j] << " > min(";
                // std::cout << "draught[" << new_path[j] << "]: " << draught[new_path[j]] << ", draught[" << next_port << "]: " << next_port_draught;
                // std::cout << ", Q: " << ship_capacity << ")" << std::endl;
                return false;
            }
        }
    
        new_path[y+1] = n+i;
        new_partial_load[y+1] = new_partial_load[y] + demand[n+i];
        // std::cout << new_path[y+1] << "(" << new_partial_load[y+1] << ") ";
    
        if(new_partial_load[y+1] > std::min(std::min(draught[n+i], draught[path[y]]), ship_capacity)) {
            // std::cout << std::endl << "\t\t\t\tInfeasible: new partial load at " << new_path[y+1] << " is " << new_partial_load[y+1] << " > min(";
            // std::cout << "draught[" << n+i << "]: " << draught[n+i] << ", draught[" << path[y] << "]: " << draught[path[y]];
            // std::cout << ", Q: " << ship_capacity << ")" << std::endl;
            return false;
        }
    
        for(int j = y + 2; j < path.size() + 2; j++) {
            new_path[j] = path[j-2];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j]];
        
            // std::cout << new_path[j] << "(" << new_partial_load[j] << ") ";
        
            int next_port = (j <= path.size() ? path[j-1] : -1);
            int next_port_draught = (j <= path.size() ? draught[path[j-1]] : std::numeric_limits<int>::max());
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                // std::cout << std::endl << "\t\t\t\tInfeasible: new partial load at " << new_path[j] << " is " << new_partial_load[j] << " > min(";
                // std::cout << "draught[" << new_path[j] << "]: " << draught[new_path[j]] << ", draught[" << next_port << "]: " << next_port_draught;
                // std::cout << ", Q: " << ship_capacity << ")" << std::endl;
                return false;
            }
        }
        // std::cout << std::endl;
    } else {
        for(int j = 0; j <= x-1; j++) {
            new_path[j] = path[j];
            new_partial_load[j] = partial_load[j];
        }
        
        new_path[x] = i;
        new_partial_load[x] = new_partial_load[x-1] + demand[i];
        
        if(new_partial_load[x] > std::min(std::min(draught[i], draught[n+i]), ship_capacity)) {
            return false;
        }
        
        new_path[x+1] = n+i;
        new_partial_load[x+1] = new_partial_load[x] + demand[n+i];
        
        if(new_partial_load[x] > std::min(std::min(draught[n+i], draught[path[x]]), ship_capacity)) {
            return false;
        }
        
        for(int j = x + 2; j < path.size() + 2; j++) {
            new_path[j] = path[j-2];
            new_partial_load[j] = new_partial_load[j-1] + demand[new_path[j-1]];
            
            int next_port = (j <= path.size() ? path[j-1] : -1);
            int next_port_draught = (j <= path.size() ? draught[path[j-1]] : std::numeric_limits<int>::max());
            if(new_partial_load[j] > std::min(std::min(draught[new_path[j]], next_port_draught), ship_capacity)) {
                return false;
            }
        }
    }
}

#endif