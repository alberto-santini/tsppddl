#include <parser/parser.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

tsp_graph parser::generate_tsp_graph() const {
    using namespace boost::property_tree;
    
    struct port {
        int pid;
        int draught;
        port() {}
        port(int pid, int draught) : pid{pid}, draught{draught} {}
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
    
    BOOST_FOREACH(const ptree::value_type& port_child, pt.get_child("ports")) {
        ports.push_back(port(port_child.second.get<int>("id"), port_child.second.get<int>("draught")));
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
    
    auto demand = tsp_graph::demand_t(2*n+2, 0);
    auto draught = tsp_graph::draught_t(2*n+2, 0);
    
    demand[0] = 0; demand[2*n+1] = 0;
    draught[0] = ports[0].draught; draught[2*n+1] = ports[0].draught;

    for(auto i = 1; i <= n; i++) {
        demand[i] = requests[i-1].demand;
        demand[n+i] = -requests[i-1].demand;
        draught[i] = ports[requests[i-1].origin].draught;
        draught[n+i] = ports[requests[i-1].destination].draught;
    }
        
    auto cost = tsp_graph::cost_t(2*n+2, tsp_graph::cost_row_t(2*n+2, -1));
    cost[0][0] = 0; cost[2*n+1][2*n+1] = 0; cost[0][2*n+1] = 0; cost[2*n+1][0] = 0;
    
    for(auto i = 1; i <= n; i++) {
        cost[0][i] = port_cost[0][requests[i-1].origin];
        cost[i][2*n+1] = port_cost[requests[i-1].origin][0];
        cost[0][n+i] = port_cost[0][requests[i-1].destination];
        cost[n+i][2*n+1] = port_cost[requests[i-1].destination][0];
        for(auto j = 1; j <= n; j++) {
            cost[i][j] = port_cost[requests[i-1].origin][requests[j-1].origin];
            cost[i][n+j] = port_cost[requests[i-1].origin][requests[j-1].destination];
            cost[n+i][j] = port_cost[requests[i-1].destination][requests[j-1].origin];
            cost[n+i][n+j] = port_cost[requests[i-1].destination][requests[j-1].destination];
        }
    }
    
    return tsp_graph(demand, draught, cost, capacity);
}

program_params  parser::read_program_params() const {
    using namespace boost::property_tree;
    
    auto pt = ptree();
    read_json(params_file_name, pt);
    
    auto instance_size_limits = k_opt_params::k_opt_limits();
    BOOST_FOREACH(const ptree::value_type& limit_pair, pt.get_child("k_opt.instance_size_limit")) {
        instance_size_limits.push_back(k_opt_params::k_opt_limit(limit_pair.second.get<unsigned int>("k"), limit_pair.second.get<unsigned int>("n")));
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
            pt.get<unsigned int>("subgradient.iter_reduce_theta"),
            pt.get<double>("subgradient.theta_reduce_fact"),
            pt.get<unsigned int>("subgradient.max_iter"),
            pt.get<std::string>("subgradient.results_dir")
        ),
        branch_and_cut_params(
            pt.get<bool>("branch_and_cut.two_cycles_elim"),
            pt.get<bool>("branch_and_cut.subpath_elim"),
            pt.get<bool>("branch_and_cut.print_relaxation_graph"),
            pt.get<std::string>("branch_and_cut.results_dir"),
            branch_and_cut_params::valid_inequality_with_memory_info(
                pt.get<unsigned int>("branch_and_cut.subtour_elim_valid_ineq.cut_every_n_nodes"),
                pt.get<bool>("branch_and_cut.subtour_elim_valid_ineq.enabled"),
                pt.get<bool>("branch_and_cut.subtour_elim_valid_ineq.memory")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<unsigned int>("branch_and_cut.generalised_order_valid_ineq.cut_every_n_nodes"),
                pt.get<bool>("branch_and_cut.generalised_order_valid_ineq.enabled")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<unsigned int>("branch_and_cut.capacity_valid_ineq.cut_every_n_nodes"),
                pt.get<bool>("branch_and_cut.capacity_valid_ineq.enabled")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<unsigned int>("branch_and_cut.simplified_fork_valid_ineq.cut_every_n_nodes"),
                pt.get<bool>("branch_and_cut.simplified_fork_valid_ineq.enabled")
            ),
            branch_and_cut_params::valid_inequality_info(
                pt.get<unsigned int>("branch_and_cut.fork_valid_ineq.cut_every_n_nodes"),
                pt.get<bool>("branch_and_cut.fork_valid_ineq.enabled")
            )
        )
    );
}