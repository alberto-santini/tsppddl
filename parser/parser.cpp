#include <parser/parser.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

Graph Parser::generate_graph() const {
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
    auto port_cost = cost_t();
    
    BOOST_FOREACH(const ptree::value_type& port_child, pt.get_child("ports")) {
        ports.push_back(port(port_child.second.get<int>("id"), port_child.second.get<int>("draught")));
    }
        
    BOOST_FOREACH(const ptree::value_type& request_child, pt.get_child("requests")) {
        requests.push_back(request(request_child.second.get<int>("origin"), request_child.second.get<int>("destination"), request_child.second.get<int>("demand")));
    }
    
    BOOST_FOREACH(const ptree::value_type& cost_row, pt.get_child("distances")) {
        cost_row_t port_cost_row;
        BOOST_FOREACH(const ptree::value_type& cost_val, cost_row.second.get_child("")) {
            port_cost_row.push_back(cost_val.second.get<cost_val_t>(""));
        }
        port_cost.push_back(port_cost_row);
    }
    
    auto demand = demand_t(2*n+2, 0);
    auto draught = draught_t(2*n+2, 0);
    
    demand[0] = 0; demand[2*n+1] = 0;
    draught[0] = ports[0].draught; draught[2*n+1] = ports[0].draught;

    for(auto i = 1; i <= n; i++) {
        demand[i] = requests[i-1].demand;
        demand[n+i] = -requests[i-1].demand;
        draught[i] = ports[requests[i-1].origin].draught;
        draught[n+i] = ports[requests[i-1].destination].draught;
    }
        
    auto cost = cost_t(2*n+2, cost_row_t(2*n+2, -1));
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
    
    return Graph(demand, draught, cost, capacity);
}

ProgramParams Parser::read_program_params() const {
    using namespace boost::property_tree;
    
    auto pt = ptree();
    read_json(params_file_name, pt);
    
    return ProgramParams(
        SubgradientParams(
            pt.get<bool>("subgradient.relax_mtz"),
            pt.get<bool>("subgradient.relax_prec"),
            pt.get<double>("subgradient.initial_lambda"),
            pt.get<double>("subgradient.initial_mu"),
            pt.get<unsigned int>("subgradient.iter_reduce_theta"),
            pt.get<double>("subgradient.theta_reduce_fact"),
            pt.get<unsigned int>("subgradient.max_iter"),
            pt.get<std::string>("subgradient.results_dir")
        ),
        BranchAndCutParams(
            pt.get<unsigned int>("branch_and_cut.cut_every_n_nodes"),
            pt.get<bool>("branch_and_cut.two_cycles_elim"),
            pt.get<bool>("branch_and_cut.subpath_elim"),
            pt.get<bool>("branch_and_cut.subtour_sep_memory"),
            pt.get<bool>("branch_and_cut.separate_subtour_elimination"),
            pt.get<bool>("branch_and_cut.separate_precedence"),
            pt.get<bool>("branch_and_cut.separate_capacity"),
            pt.get<bool>("branch_and_cut.separate_simplified_fork"),
            pt.get<bool>("branch_and_cut.print_relaxation_graph"),
            pt.get<std::string>("branch_and_cut.results_dir")
        )
    );
}