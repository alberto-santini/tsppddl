#ifndef CALLBACKS_HELPER_H
#define CALLBACKS_HELPER_H

#include <network/tsp_graph.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <boost/container/vector.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/range/numeric.hpp>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace callbacks_helper {
    struct solution {
        bool is_integer;
        std::vector<std::vector<double>> x;
        
        solution(bool i, const std::vector<std::vector<double>>& x) : is_integer{i}, x{x} {}
    };
}

namespace ch = callbacks_helper;

#endif