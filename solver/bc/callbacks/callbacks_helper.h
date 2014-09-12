#ifndef CALLBACKS_HELPER_H
#define CALLBACKS_HELPER_H

#include <network/graph.h>

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#include <memory>
#include <vector>

namespace CallbacksHelper {
    struct solution {
        bool is_integer;
        std::vector<std::vector<double>> x;
        
        solution(bool i, std::vector<std::vector<double>> x) : is_integer{i}, x{x} {}
    };
}

#endif