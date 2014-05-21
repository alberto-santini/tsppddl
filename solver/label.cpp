#ifndef LABEL_CPP
#define LABEL_CPP

#include <cmath>
#include <limits>
#include <iostream>

#include <solver/label.h>

bool Label::operator==(const Label& other) const {
    return (    load == other.load &&
                started_requests == other.started_requests &&
                open_requests == other.open_requests &&
                std::abs(cost - other.cost) < std::numeric_limits<double>::epsilon());
}

bool Label::operator<(const Label& other) const {
    bool strict = false;
    if( load < other.load ||
        started_requests < other.started_requests ||
        open_requests < other.open_requests) {
        strict = true;
    }
    
    return (    load <= other.load &&
                started_requests <= other.started_requests &&
                open_requests <= other.open_requests &&
                other.cost - cost >= std::numeric_limits<double>::epsilon() &&
                strict);
}

bool LabelExtender::operator()(const BoostGraph& g, Label& new_label, const Label& label, Edge e) const {
    std::shared_ptr<Node> destination = g[target(e, g)];
    int n = g[graph_bundle].num_requests;
    
    // std::cout << "Extending label from " << g[source(e, g)]->id << " to " << destination->id << std::endl;
    
    new_label.load = label.load + destination->demand;
    if(destination->id == 2 * n + 1) {
        new_label.started_requests = label.started_requests;
        new_label.open_requests = label.open_requests;
    } else if(destination->id <= n) {
        new_label.started_requests = label.started_requests + 1;
        new_label.open_requests = label.open_requests + 1;
    } else if(destination->id > n) {
        new_label.started_requests = label.started_requests;
        new_label.open_requests = label.open_requests - 1;
    }
    new_label.cost = label.cost + g[e]->cost;
    
    if(new_label.load > std::min(g[graph_bundle].ship_capacity, destination->port->draught)) {
        // std::cout << "\tLabel infeasible due to draught or capacity limits" << std::endl;
        return false;
    }
    
    if( destination->id > 0 &&
        destination->id <= n &&
        new_label.started_requests > n) {
        // std::cout << "\tLabel to o(" << destination->id << ") infeasible: L = " << label.load << ", V = " << new_label.started_requests << ", O = " << new_label.open_requests << std::endl;
        return false;
    }
    
    if( destination->id > n &&
        destination->id <= 2 * n &&
        (new_label.open_requests < 0 || new_label.load < 0)) {
        // std::cout << "\tLabel to d(" << destination->id - n << ") infeasible: L = " << label.load << ", V = " << new_label.started_requests << ", O = " << new_label.open_requests << std::endl;
        return false;
    }
    
    if( destination->id == 2 * n + 1 &&
        (new_label.started_requests < n || new_label.open_requests > 0)) {
        // std::cout << "\tLabel to 2n+1 infeasible: L = " << label.load << ", V = " << new_label.started_requests << ", O = " << new_label.open_requests << std::endl;
        return false;
    }
    
    // std::cout << "\tExtension feasible" << std::endl;
    
    return true;
}

bool LabelDominator::operator()(const Label& l1, const Label& l2) const {
    if(l1 == l2 || l1 < l2) {
        // std::cout << "\tLabel (" << l1.load << ", " << l1.started_requests << ", " << l1.open_requests << ", " << l1.cost;
        // std::cout << ") dominates label (" << l2.load << ", " << l2.started_requests << ", " << l2.open_requests << ", " << l2.cost;
        // std::cout << ")" << std::endl;
        return true;
    } else {
        // std::cout << "\tLabel (" << l1.load << ", " << l1.started_requests << ", " << l1.open_requests << ", " << l1.cost;
        // std::cout << ") can't dominate label (" << l2.load << ", " << l2.started_requests << ", " << l2.open_requests << ", " << l2.cost;
        // std::cout << ")" << std::endl;
        return false;
    }
}

#endif