#ifndef CALLBACKS_HELPER_H
#define CALLBACKS_HELPER_H

#include <network/graph.h>

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

typedef boost::container::vector<bool> bvec;
typedef std::vector<int> ivec;

namespace CallbacksHelper {
    struct solution {
        bool is_integer;
        std::vector<std::vector<double>> x;
        
        solution(bool i, const std::vector<std::vector<double>>& x) : is_integer{i}, x{x} {}
    };
    
    struct sets_info {
        int n;
        bvec in_S;
        bvec in_tabu;
        ivec tabu_start;
        bvec in_fs; double fs;  // First sum
        bvec in_ss; double ss;  // Second sum
        bvec in_ts; double ts;  // Third sum
        double lhs;
        
        sets_info() {}
        
        sets_info(int n, const bvec& in_S, const bvec& in_tabu, const ivec& tabu_start, const bvec& in_fs, double fs, const bvec& in_ss, double ss, const bvec& in_ts, double ts, double lhs) : n{n}, in_S{in_S}, in_tabu{in_tabu}, tabu_start{tabu_start}, in_fs{in_fs}, fs{fs}, in_ss{in_ss}, ss{ss}, in_ts{in_ts}, ts{ts}, lhs{lhs} {}
        
        // Methods for writing easier to read for(...) loops
        bool is_in_S(int i) const;
        bool is_in_fs(int i) const;
        bool is_in_ss(int i) const;
        bool is_in_ts(int i) const;
        
        bool empty_S() const;
        int first_non_tabu() const;
        void print_S(std::ostream& os) const;
        void print_fs(std::ostream& os) const;
        void print_ss(std::ostream& os) const;
        void print_ts(std::ostream& os) const;
        
    private:
        void print_set(std::ostream& os, const bvec& set) const;
    };
}

namespace ch = CallbacksHelper;

#endif