#ifndef HEURISTIC_SOLVER_CPP
#define HEURISTIC_SOLVER_CPP

#include <heuristics/two_phases_heuristic.h>
#include <heuristics/max_regret_heuristic.h>
#include <heuristics/heuristic_solver.h>

HeuristicSolver::HeuristicSolver(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph) : data(data), graph(graph) {
    rd = std::make_shared<RawData>(data, graph);
}

GenericPath HeuristicSolver::solve_two_phases_min_distance() const {
    auto request_pricer = [this] (const int r) -> int {
        return (rd->d[r][r+rd->n]);
    };
    
    auto request_comparator = [this, request_pricer] (const int r1, const int r2) -> bool {
        return (request_pricer(r1) < request_pricer(r2));
    };
    
    auto insertion_pricer = [this] (const int cost, const int load) -> double {
        return ((double) cost);
    };
    
    auto insertion_comparator = [this, insertion_pricer] (const int cost1, const int load1, const int cost2, const int load2) -> bool {
        return (insertion_pricer(cost1, load1) < insertion_pricer(cost2, load2));
    };
    
    TwoPhasesHeuristic<decltype(request_pricer), decltype(request_comparator), decltype(insertion_pricer), decltype(insertion_comparator)> tph(rd, request_pricer, request_comparator, insertion_pricer, insertion_comparator);
    return tph.solve();
}

GenericPath HeuristicSolver::solve_two_phases_max_distance() const {
    auto request_pricer = [this] (const int r) -> int {
        return (rd->d[r][r+rd->n]);
    };
    
    auto request_comparator = [this, request_pricer] (const int r1, const int r2) -> bool {
        return (request_pricer(r1) > request_pricer(r2));
    };
    
    auto insertion_pricer = [this] (const int cost, const int load) -> double {
        return ((double) cost);
    };
    
    auto insertion_comparator = [this, insertion_pricer] (const int cost1, const int load1, const int cost2, const int load2) -> bool {
        return (insertion_pricer(cost1, load1) < insertion_pricer(cost2, load2));
    };
    
    TwoPhasesHeuristic<decltype(request_pricer), decltype(request_comparator), decltype(insertion_pricer), decltype(insertion_comparator)> tph(rd, request_pricer, request_comparator, insertion_pricer, insertion_comparator);
    return tph.solve();
}

GenericPath HeuristicSolver::solve_max_regret_max_load_over_distance() const {
    auto insertion_pricer = [this] (const int cost, const int load) -> double {
        return ((double)load / (double)cost);
    };
    
    auto insertion_comparator = [this, insertion_pricer] (const int cost1, const int load1, const int cost2, const int load2) -> bool {
        return (insertion_pricer(cost1, load1) > insertion_pricer(cost2, load2));
    };
    
    auto regret_calculator = [this, insertion_pricer] (const int best_cost, const int best_load, const int second_best_cost, const int second_best_load) -> double {
        return (insertion_pricer(best_cost, best_load) - insertion_pricer(second_best_cost, second_best_load));
    };
    
    MaxRegretHeuristic<decltype(insertion_pricer), decltype(insertion_comparator), decltype(regret_calculator)> mrh(rd, insertion_pricer, insertion_comparator, regret_calculator);
    return mrh.solve();
}

GenericPath HeuristicSolver::solve_max_regret_min_load_times_distance() const {
    auto insertion_pricer = [this] (const int cost, const int load) -> double {
        return ((double)load * (double)cost);
    };
    
    auto insertion_comparator = [this, insertion_pricer] (const int cost1, const int load1, const int cost2, const int load2) -> bool {
        if(insertion_pricer(cost1, load1) == 0) {
            return false;
        }
        
        if(insertion_pricer(cost2, load2) == 0) {
            return true;
        }

        return (insertion_pricer(cost1, load1) < insertion_pricer(cost2, load2));
    };
    
    auto regret_calculator = [this, insertion_pricer] (const int best_cost, const int best_load, const int second_best_cost, const int second_best_load) -> double {
        return (abs(insertion_pricer(second_best_cost, second_best_load) - insertion_pricer(best_cost, best_load)));
    };
    
    MaxRegretHeuristic<decltype(insertion_pricer), decltype(insertion_comparator), decltype(regret_calculator)> mrh(rd, insertion_pricer, insertion_comparator, regret_calculator);
    return mrh.solve();
}

#endif