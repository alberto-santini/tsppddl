#ifndef INSERTER_H
#define INSERTER_H

#include <network/tsp_graph.h>
#include <network/path.h>

#include <utility>
#include <limits>
#include <iostream>

template<class IS>
struct inserter {
    using result = std::tuple<bool, double, path>;
    
    const IS& ins_scorer;
    
    inserter(const IS& ins_scorer) : ins_scorer(ins_scorer) {}
    virtual result operator()(const tsp_graph& g, const path& old_path, int request) const = 0;
};

template<class IS>
struct normal_inserter : inserter<IS> {
    normal_inserter(const IS& insertion_scorer) : inserter<IS>(insertion_scorer) {}
    typename inserter<IS>::result operator()(const tsp_graph& g, const path& old_path, int request) const;
};

template<class IS>
typename inserter<IS>::result normal_inserter<IS>::operator()(const tsp_graph& g, const path& old_path, int request) const {
    bool overall_success;
    double best_score = std::numeric_limits<double>::min();
    path best_path;
    
    for(auto orig_position = 1u; orig_position < old_path.length(); ++orig_position) {
        for(auto dest_position = orig_position; dest_position < old_path.length(); ++dest_position) {
            bool success;
            double new_score;
            path new_path;
            
            std::tie(success, new_score, new_path) = this->ins_scorer(g, old_path, request, orig_position, dest_position);
            
            if(success && new_score > best_score) {
                best_score = new_score;
                best_path = new_path;
                overall_success = true;
            }
        }
    }
    
    return std::make_tuple(overall_success, best_score, best_path);
}

template<class IS>
struct max_regret_inserter : inserter<IS> {
    max_regret_inserter(const IS& insertion_scorer) : inserter<IS>(insertion_scorer) {}
    typename inserter<IS>::result operator()(const tsp_graph& g, const path& old_path, int request) const;
};

template<class IS>
typename inserter<IS>::result max_regret_inserter<IS>::operator()(const tsp_graph& g, const path& old_path, int request) const {
    bool overall_success;
    double best_score = std::numeric_limits<double>::min(), second_best_score = std::numeric_limits<double>::min();
    path best_path, second_best_path;
    
    for(auto orig_position = 1u; orig_position < old_path.length(); ++orig_position) {
        for(auto dest_position = orig_position; dest_position < old_path.length(); ++dest_position) {
            bool success;
            double new_score;
            path new_path;
            
            std::tie(success, new_score, new_path) = this->ins_scorer(g, old_path, request, orig_position, dest_position);
            
            if(success && new_score > second_best_score) {
                second_best_score = new_score;
                second_best_path = new_path;
                overall_success = true;
                
                if(second_best_score > best_score) {
                    std::swap(second_best_score, best_score);
                    std::swap(second_best_path, best_path);
                }
            }
        }
    }
    
    if(overall_success) {
        if(second_best_score > std::numeric_limits<double>::min()) {
            return std::make_tuple(overall_success, best_score - second_best_score, best_path);
        }
    }
    
    return std::make_tuple(overall_success, best_score, best_path);
}

#endif