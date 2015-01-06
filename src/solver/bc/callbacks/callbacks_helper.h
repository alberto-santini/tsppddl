#ifndef CALLBACKS_HELPER_H
#define CALLBACKS_HELPER_H

#include <network/tsp_graph.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <boost/container/vector.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/range/numeric.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

namespace callbacks_helper {
    struct solution {
        bool is_integer;
        std::vector<std::vector<double>> x;
        
        solution(bool i, const std::vector<std::vector<double>>& x) : is_integer{i}, x{x} {}
    };
    
    inline double eps(double base_val) {
        auto _base_val = std::abs(base_val);
        
        // If _base_val is 0, transform it into 1 for the purpose of taking the log
        if(_base_val < std::numeric_limits<double>::epsilon() && _base_val > - std::numeric_limits<double>::epsilon()) {
            _base_val = 1.0;
        }
        
        // Epsilon is 5 orders of magnitude smaller than the value
        return std::pow(10, std::floor(std::log10(_base_val)) - 5);
    }
}

namespace ch = callbacks_helper;

#endif