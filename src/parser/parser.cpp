#include <parser/parser.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <limits>

tsp_graph parser::generate_tsp_graph() const {
    using namespace boost::property_tree;
    
    struct port {
        int     pid;
        int     draught;
        bool    depot;
        port() {}
        port(int pid, int draught, bool depot) : pid{pid}, draught{draught}, depot{depot} {}
    };
    
    struct request {
        int origin;
        int destination;
        int demand;
        request() {}
        request(int origin, int destination, int demand) : origin{origin}, destination{destination}, demand{demand} {}
    };
        
    auto pt = ptree();
    read_json(instance_file_name, pt);
    
    auto n = pt.get<int>("num_requests");
    auto capacity = pt.get<int>("capacity");
    
    auto ports = std::vector<port>();
    auto requests = std::vector<request>();
    auto port_cost = tsp_graph::cost_t();
    auto depot_id = -1;
    
    BOOST_FOREACH(const ptree::value_type& port_child, pt.get_child("ports")) {
        auto port_id = port_child.second.get<int>("id");
        auto is_depot = port_child.second.get<bool>("depot");
        auto draught = port_child.second.get<int>("draught");
        if(is_depot) {
            if(depot_id == -1) {
                depot_id = port_id;
            } else {
                throw std::runtime_error("There is an instance with two depots!");
            }
        }
        ports.push_back(port(port_id, (is_depot ? std::numeric_limits<int>::max() : draught), is_depot));
    }
        
    BOOST_FOREACH(const ptree::value_type& request_child, pt.get_child("requests")) {
        requests.push_back(request(request_child.second.get<int>("origin"), request_child.second.get<int>("destination"), request_child.second.get<int>("demand")));
    }
    
    BOOST_FOREACH(const ptree::value_type& cost_row, pt.get_child("distances")) {
        tsp_graph::cost_row_t port_cost_row;
        BOOST_FOREACH(const ptree::value_type& cost_val, cost_row.second.get_child("")) {
            port_cost_row.push_back(cost_val.second.get<tsp_graph::cost_val_t>(""));
        }
        port_cost.push_back(port_cost_row);
    }
    
    if(depot_id == -1) {
        throw std::runtime_error("There is an instance without depot!");
    }
     
    auto demand = tsp_graph::demand_t(2*n+2, 0);
    auto draught = tsp_graph::draught_t(2*n+2, 0);
    
    demand[0] = 0; demand[2*n+1] = 0;
    draught[0] = ports[depot_id].draught; draught[2*n+1] = ports[depot_id].draught;

    for(auto i = 1; i <= n; i++) {
        demand[i] = requests[i-1].demand;
        demand[n+i] = -requests[i-1].demand;
        draught[i] = ports[requests[i-1].origin].draught;
        draught[n+i] = ports[requests[i-1].destination].draught;
    }
        
    auto cost = tsp_graph::cost_t(2*n+2, tsp_graph::cost_row_t(2*n+2, -1));
    
    for(auto i = 1; i <= n; i++) {
        cost[0][i] = port_cost[depot_id][requests[i-1].origin];
        cost[n+i][2*n+1] = port_cost[requests[i-1].destination][depot_id];
        for(auto j = 1; j <= n; j++) {
            cost[i][j] = port_cost[requests[i-1].origin][requests[j-1].origin];
            cost[i][n+j] = port_cost[requests[i-1].origin][requests[j-1].destination];
            cost[n+i][j] = port_cost[requests[i-1].destination][requests[j-1].origin];
            cost[n+i][n+j] = port_cost[requests[i-1].destination][requests[j-1].destination];
        }
    }
    
    return tsp_graph(demand, draught, cost, capacity, instance_file_name);
}

program_params  parser::read_program_params() const {
    using namespace boost::property_tree;
    
    auto pt = ptree();
    read_json(params_file_name, pt);
    
    auto instance_size_limits = k_opt_params::k_opt_limits();
    BOOST_FOREACH(const ptree::value_type& limit_pair, pt.get_child("k_opt.instance_size_limit")) {
        instance_size_limits.push_back(k_opt_params::k_opt_limit(limit_pair.second.get<int>("k"), limit_pair.second.get<int>("n")));
    }
    
    auto tabu_tuning_list_size = std::vector<int>();
    BOOST_FOREACH(const ptree::value_type& lsize, pt.get_child("tabu_tuning.tabu_list_size")) {
        tabu_tuning_list_size.push_back(lsize.second.get<int>(""));
    }
    
    return program_params(
        k_opt_params(
            instance_size_limits
        ),
        subgradient_params(
            pt.get<bool>("subgradient.relax_mtz"),
            pt.get<bool>("subgradient.relax_prec"),
            pt.get<double>("subgradient.initial_lambda"),
            pt.get<double>("subgradient.initial_mu"),
            pt.get<int>("subgradient.iter_reduce_theta"),
            pt.get<double>("subgradient.theta_reduce_fact"),
            pt.get<int>("subgradient.max_iter"),
            pt.get<std::string>("subgradient.results_dir")
        ),
        branch_and_cut_params(
            pt.get<bool>("branch_and_cut.two_cycles_elim"),
            pt.get<bool>("branch_and_cut.subpath_elim"),
            pt.get<int>("branch_and_cut.max_infeas_subpaths"),
            pt.get<bool>("branch_and_cut.print_relaxation_graph"),
            pt.get<bool>("branch_and_cut.use_initial_solutions"),
            pt.get<std::string>("branch_and_cut.results_dir"),
            branch_and_cut_params::valid_inequality_with_memory_info(
                pt.get<int>("branch_and_cut.subtour_elim_valid_ineq.n1"),
                pt.get<int>("branch_and_cut.subtour_elim_valid_ineq.n2"),
                pt.get<double>("branch_and_cut.subtour_elim_valid_ineq.p1"),
                pt.get<double>("branch_and_cut.subtour_elim_valid_ineq.p2"),
                pt.get<double>("branch_and_cut.subtour_elim_valid_ineq.p3"),
                pt.get<bool>("branch_and_cut.subtour_elim_valid_ineq.enabled"),
                pt.get<bool>("branch_and_cut.subtour_elim_valid_ineq.memory")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<int>("branch_and_cut.generalised_order_valid_ineq.n1"),
                pt.get<int>("branch_and_cut.generalised_order_valid_ineq.n2"),
                pt.get<double>("branch_and_cut.generalised_order_valid_ineq.p1"),
                pt.get<double>("branch_and_cut.generalised_order_valid_ineq.p2"),
                pt.get<double>("branch_and_cut.generalised_order_valid_ineq.p3"),
                pt.get<bool>("branch_and_cut.generalised_order_valid_ineq.enabled")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<int>("branch_and_cut.capacity_valid_ineq.n1"),
                pt.get<int>("branch_and_cut.capacity_valid_ineq.n2"),
                pt.get<double>("branch_and_cut.capacity_valid_ineq.p1"),
                pt.get<double>("branch_and_cut.capacity_valid_ineq.p2"),
                pt.get<double>("branch_and_cut.capacity_valid_ineq.p3"),
                pt.get<bool>("branch_and_cut.capacity_valid_ineq.enabled")
            ),
            branch_and_cut_params::valid_inequality_with_lifted_version_info(
                pt.get<int>("branch_and_cut.fork_valid_ineq.n1"),
                pt.get<int>("branch_and_cut.fork_valid_ineq.n2"),
                pt.get<double>("branch_and_cut.fork_valid_ineq.p1"),
                pt.get<double>("branch_and_cut.fork_valid_ineq.p2"),
                pt.get<double>("branch_and_cut.fork_valid_ineq.p3"),
                pt.get<bool>("branch_and_cut.fork_valid_ineq.enabled"),
                pt.get<bool>("branch_and_cut.fork_valid_ineq.lifted_version_enabled")
            )
        ),
        tabu_search_params(
            pt.get<int>("tabu_search.tabu_list_size"),
            pt.get<int>("tabu_search.max_iter"),
            pt.get<int>("tabu_search.max_iter_without_improving"),
            pt.get<int>("tabu_search.max_parallel_searches"),
            pt.get<std::string>("tabu_search.results_dir"),
            pt.get<bool>("tabu_search.track_progress"),
            pt.get<std::string>("tabu_search.progress_results_dir")
        ),
        tabu_search_tuning_params(
            tabu_tuning_list_size
        ),
        constructive_heuristics_params(
            pt.get<bool>("constructive_heuristics.print_solutions"),
            pt.get<std::string>("constructive_heuristics.results_dir"),
            pt.get<std::string>("constructive_heuristics.solutions_dir")
        ),
        pt.get<int>("cplex_threads"),
        pt.get<int>("cplex_timeout")
    );
}