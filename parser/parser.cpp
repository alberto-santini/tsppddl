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
        port(const int pid, const int draught) : pid{pid}, draught{draught} {}
    };
    
    struct request {
        int origin;
        int destination;
        int demand;
        request() {}
        request(const int origin, const int destination, const int demand) : origin{origin}, destination{destination}, demand{demand} {}
    };
        
    ptree pt;
    read_json(file_name, pt);
    
    int n {pt.get<int>("num_requests")};
    int capacity {pt.get<int>("capacity")};
    
    std::vector<port> ports;
    std::vector<request> requests;
    cost_t port_cost;
    
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
        
    demand_t demand(2*n+2, 0);
    draught_t draught(2*n+2, 0);
    demand[0] = 0; demand[2*n+1] = 0;
    draught[0] = ports[0].draught; draught[2*n+1] = ports[0].draught;

    for(int i = 1; i <= n; i++) {
        demand[i] = requests[i-1].demand;
        demand[n+i] = -requests[i-1].demand;
        draught[i] = ports[requests[i-1].origin].draught;
        draught[n+i] = ports[requests[i-1].destination].draught;
    }
        
    cost_t cost(2*n+2, cost_row_t(2*n+2, -1));
    cost[0][0] = 0; cost[2*n+1][2*n+1] = 0; cost[0][2*n+1] = 0; cost[2*n+1][0] = 0;
    
    for(int i = 1; i <= n; i++) {
        cost[0][i] = port_cost[0][requests[i-1].origin];
        cost[i][2*n+1] = port_cost[requests[i-1].origin][0];
        cost[0][n+i] = port_cost[0][requests[i-1].destination];
        cost[n+i][2*n+1] = port_cost[requests[i-1].destination][0];
        for(int j = 1; j <= n; j++) {
            cost[i][j] = port_cost[requests[i-1].origin][requests[j-1].origin];
            cost[i][n+j] = port_cost[requests[i-1].origin][requests[j-1].destination];
            cost[n+i][j] = port_cost[requests[i-1].destination][requests[j-1].origin];
            cost[n+i][n+j] = port_cost[requests[i-1].destination][requests[j-1].destination];
        }
    }
    
    return Graph {demand, draught, cost, capacity};
}