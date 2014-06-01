#ifndef RAW_DATA_H
#define RAW_DATA_H

#include <network/graph.h>
#include <parser/data.h>

class RawData {
public:
    int n; // Number of requests
    int Q; // Ship's capacity
    std::vector<std::vector<int>> d; // N.B. in TSPLIB distances are always integers
    std::vector<int> draught; // N.B. in our instances draughts are always integers
    std::vector<int> demand; // N.B. in our instances demands are always integers
    
    RawData(const std::shared_ptr<Data> data, const std::shared_ptr<Graph> graph);
};

#endif