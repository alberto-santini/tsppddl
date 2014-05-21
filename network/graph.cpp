#ifndef GRAPH_CPP
#define GRAPH_CPP

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <utility>

#include <network/graph.h>

Graph::Graph(std::shared_ptr<Data> data) {
    std::cout << "Creating graph" << std::endl;
    
    data->check_port_ids_consistent_with_depot();
    std::cout << "\tPort ids are consistent" << std::endl;
    
    name = "Graph for " + data->data_file_name;
    g[graph_bundle].num_requests = data->num_requests;
    g[graph_bundle].ship_capacity =  data->capacity;
    
    int n = data->num_requests;
    
    handy_dt = std::vector<std::vector<double>>();
    
    std::cout << "\tCreated graph with name: " << name << std::endl;
    
    Vertex start_depot, end_depot;
    std::shared_ptr<Port> depot = data->port_by_id(0);
    
    start_depot = add_vertex(g);
    g[start_depot] = std::make_shared<Node>(0, depot, 0);
    
    end_depot = add_vertex(g);
    g[end_depot] = std::make_shared<Node>(2 * n + 1, depot, 0);
    
    std::cout << "\tDepots created" << std::endl;
    
    int node_id = 1;
    
    for(const std::shared_ptr<Request>& request : data->requests) {
        Vertex request_origin, request_destination;
        
        request_origin = add_vertex(g);
        g[request_origin] = std::make_shared<Node>(node_id, request->origin, request->demand);
        
        request_destination = add_vertex(g);
        g[request_destination] = std::make_shared<Node>(n + node_id, request->destination, - request->demand);
        
        node_id++;
    }
    
    std::cout << "\tNodes created" << std::endl;
    
    int arc_id = 0;
    
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        std::shared_ptr<Node> i = g[*vi];
        vit vj, vj_end;
        
        if(i->id == 2 * n + 1) {
            continue; // No arcs out of 2n+1
        }
        
        for(std::tie(vj, vj_end) = vertices(g); vj != vj_end; ++vj) {
            std::shared_ptr<Node> j = g[*vj];
            
            if(i->id == j->id) {
                continue; // No arcs i -> i
            }
            
            if(j->id == 0) {
                continue; // No arcs to 0
            }
            
            if(i->id == 0 && j->id > n) {
                continue; // No arcs 0 -> n+i
            }
            
            if(i->id <= n && j->id == 2 * n + 1) {
                continue; // No arcs i -> 2n+1
            }
            
            if(i->id == j->id + n) {
                continue; // No arcs n+i -> i
            }
            
            if( (i->id <= n && i->demand > std::min(std::min(i->port->draught, j->port->draught), g[graph_bundle].ship_capacity)) ||
                (i->id <= n && i->demand + j->demand > std::min(j->port->draught, g[graph_bundle].ship_capacity)) ||
                (i->id > n && -i->demand > std::min(std::min(i->port->draught, j->port->draught), g[graph_bundle].ship_capacity)) ||
                (i->id > n && -i->demand - j->demand > std::min(j->port->draught, g[graph_bundle].ship_capacity))) {
                    continue; // No arcs violating capacity or draught constraints
                }
            
            double distance = data->distances[i->port->id][j->port->id];
            Edge e = add_edge(*vi, *vj, g).first;
            g[e] = std::make_shared<Arc>(arc_id++, distance);
        }
    }
    
    std::cout << "\tArcs created" << std::endl;
    
    generate_distance_table();
    
    std::cout << "\tDistance table generated" << std::endl;
}

void Graph::generate_distance_table() {
    int n = g[graph_bundle].num_requests;
    handy_dt = std::vector<std::vector<double>>(2 * n + 2, std::vector<double>(2 * n + 2, -1.0));
    for(int i = 0; i <= 2 * n + 1; i++) {
        for(int j = 0; j <= 2 * n + 1; j++) {
            eit ei, ei_end;
            for(std::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
                if(g[source(*ei, g)]->id == i && g[target(*ei, g)]->id == j) {
                    handy_dt[i][j] = g[*ei]->cost;
                    break;
                }
            }
        }
    }
}

void Graph::print_data() const {
    std::cout << "Graph: " << name << std::endl;
    std::cout << "\tNodes: " << num_vertices(g) << std::endl;
    // vit vi, vi_end;
    // for(std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    //     std::cout << "\t\t" << g[*vi]->id << std::endl;
    // }
    std::cout << "\tArcs: " << num_edges(g) << std::endl;
    // eit ei, ei_end;
    // for(std::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    //     std::cout << "\t\t(" << g[source(*ei, g)]->id << ", " << g[target(*ei, g)]->id << ")" << std::endl;
    // }
}

void Graph::print_graphviz(const std::string file_name) const {
    std::ofstream file;
    file.open(file_name, std::ios::out | std::ios::trunc);
    
    if(file.is_open()) {
        write_graphviz(file, g, VertexLabelWriter(this), EdgeLabelWriter(this), GraphLabelWriter(), VertexIndexMap(this));
    } else {
        throw std::runtime_error("Can't open file for output: " + file_name);
    }
    
    file.close();
}

Vertex Graph::get_vertex(const int id) const {
    vit vi, vi_end;
    for(std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        if(g[*vi]->id == id) {
            return *vi;
        }
    }
    throw std::runtime_error("Can't find vertex: " + std::to_string(id));
}

Edge Graph::get_edge(const int source_id, const int target_id) const {
    eit ei, ei_end;
    for(std::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        if(g[source(*ei, g)]->id == source_id && g[target(*ei, g)]->id == target_id) {
            return *ei;
        }
    }
    throw std::runtime_error("Can't find edge: " + std::to_string(source_id) + " -> " + std::to_string(target_id));
}

Path Graph::order_path(Path up) const {
    int current_node = 0;
    Path p;
    while(up.size() > 0) {
        for(Path::iterator it = up.begin(); it != up.end(); ++it) {
            if(g[source(*it, g)]->id == current_node) {
                p.push_back(*it);
                current_node = g[target(*it, g)]->id;
                up.erase(it);
                break;
            }
        }
    }
    return p;
}

void Graph::print_path(Path p) const {
    for(const Edge& e : p) {
        std::cout << g[source(e, g)]->id << " -> ";
    }
    std::cout << g[target(p.back(), g)]->id << std::endl;
}

void Graph::print_unordered_path(Path up) const {
    print_path(order_path(up));
}

double Graph::cost(const Path& p) const {
    double cost = 0;
    for(const Edge& e : p) {
        cost += g[e]->cost;
    }
    return cost;
}

#endif