#include <parser/parser.h>
#include <program/program.h>
#include <solver/heuristics/heuristic_solver.h>
#include <solver/bc/bc_solver.h>
#include <solver/metaheuristics/tabu/tabu_solver.h>
#include <solver/subgradient/subgradient_solver.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

program::program(const std::vector<std::string>& args) {
    if(args.size() != 3) {
        print_usage();
        return;
    }
    
    load(args[1], args[0]);
    
    std::vector<std::string> possible_parameters = {
        "branch_and_cut",
        "tabu_and_branch_and_cut",
        "tabu_tuning",
        "subgradient",
        "tabu",
        "heuristics",
        "try_combinations_branch_and_cut"
    };
    
    if(std::find(possible_parameters.begin(), possible_parameters.end(), args[2]) == possible_parameters.end()) {
        print_usage();
        return;
    }
    
    auto heuristic_solutions = std::vector<path>();
    auto hsolv = heuristic_solver(g, params, data);
    
    if(args[2] == "heuristics") {
        heuristic_solutions = hsolv.run_constructive_heuristics();
    } else {
        if(params.bc.use_initial_solutions) {
            heuristic_solutions = hsolv.run_all_heuristics();
        }
    }

    if(args[2] == "branch_and_cut") {
        auto bsolv = bc_solver(g, params, data, heuristic_solutions);
        bsolv.solve_with_branch_and_cut();
    } else if(args[2] == "subgradient") {        
        auto ssolv = subgradient_solver(g, params, heuristic_solutions);
        ssolv.solve();
    } else if(args[2] == "tabu" || args[2] == "tabu_and_branch_and_cut" || args[2] == "try_combinations_branch_and_cut") {
        auto tsolv = tabu_solver(g, params, data, heuristic_solutions);
        auto sols = tsolv.solve_sequential();
        
        if(args[2] != "tabu") {
            heuristic_solutions.insert(heuristic_solutions.end(), sols.begin(), sols.end());
        }
        
        if(args[2] == "tabu_and_branch_and_cut") {
            auto bsolv = bc_solver(g, params, data, heuristic_solutions);
            bsolv.solve_with_branch_and_cut();
        }
    } else if(args[2] == "tabu_tuning") {
        auto tsolv = tabu_solver(g, params, data, heuristic_solutions);
        tsolv.solve_parameter_tuning();
    }
    
    if(args[2] == "try_combinations_branch_and_cut") {
        try_all_combinations_of_bc(heuristic_solutions);
    }
}

void program::try_all_combinations_of_bc(const std::vector<path>& heuristic_solutions) {
    // 1) Basic model
    params.bc.two_cycles_elim = false;
    params.bc.subpath_elim = false;
    params.bc.subtour_elim.enabled = false;
    params.bc.generalised_order.enabled = false;
    params.bc.capacity.enabled = false;
    params.bc.fork.enabled = false;
    params.bc.fork.lifted = false;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 2) Enable ONLY 2-cycle elimination
    params.bc.two_cycles_elim = true;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 3) Enable ONLY subpath-elimination cache
    params.bc.two_cycles_elim = false;
    params.bc.subpath_elim = true;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 4) Enable ONLY SE cuts
    params.bc.subpath_elim = false;
    params.bc.subtour_elim.enabled = true;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 5) Enable ONLY GO cuts
    params.bc.subtour_elim.enabled = false;
    params.bc.generalised_order.enabled = true;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 6) Enable ONLY CAP cuts
    params.bc.generalised_order.enabled = false;
    params.bc.capacity.enabled = true;
    params.bc.capacity.cut_every_n_nodes = 1;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 6-bis) Enable ONLY CAP cuts every 50 nodes
    params.bc.capacity.cut_every_n_nodes = 50;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 7) Enable ONLY FORK cuts
    params.bc.capacity.enabled = false;
    params.bc.fork.enabled = true;
    params.bc.fork.cut_every_n_nodes = 1;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 7-bis) Enable ONLY FORK cuts every 50 nodes
    params.bc.fork.cut_every_n_nodes = 50;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
    
    // 8) Enable ONLY FORK, IN-FORK, OUT-FORK cuts
    params.bc.fork.cut_every_n_nodes = 1;
    params.bc.fork.lifted = true;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();

    // 8-bis) Enable ONLY FORK, IN-FORK, OUT-FORK cuts every 50 nodes
    params.bc.fork.cut_every_n_nodes = 50;
    {
        auto solv = bc_solver(g, params, data, heuristic_solutions);
        solv.solve_with_branch_and_cut();
    }
    data.reset_for_new_branch_and_cut();
}

void program::load(const std::string& params_filename, const std::string& instance_filename) {
    auto par = parser(params_filename, instance_filename);
    g = std::move(par.generate_tsp_graph());
    params = std::move(par.read_program_params());
    data = program_data();
}

void program::print_usage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t./tsppddl <instance> <params> [branch_and_cut | subgradient | tabu | heuristics | tabu_and_branch_and_cut | try_combinations_branch_and_cut]" << std::endl;
    std::cout << "\t\t instance: path to the json file with the instance data" << std::endl;
    std::cout << "\t\t params: path to the json file with the program params" << std::endl;
    std::cout << "\t\t branch_and_cut: to start solving the problem with branch and cut (warmstarted with heuristic solutions)" << std::endl;
    std::cout << "\t\t subgradient: to start solving the problem with lagrangian relaxation and the subgradient method" << std::endl;
    std::cout << "\t\t tabu: to start solving the problem with the tabu search metaheuristic algorithm" << std::endl;
    std::cout << "\t\t tabu_tuning: to start the parameter tuning for the tabu search metaheuristic algorithm" << std::endl;
    std::cout << "\t\t heuristics: to run the constructive heuristics" << std::endl;
    std::cout << "\t\t tabu_and_branch_and_cut: to start solving the problem with branch and cut (warmstarted with tabu metaheuristics solutions)" << std::endl;
    std::cout << "\t\t try_combinations_branch_and_cut: execute a calibration run with many different options for bc" << std::endl;
}