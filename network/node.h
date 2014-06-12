#ifndef NODE_H
#define NODE_H

struct Node {
    int id;
    int demand;
    int draught;
    bool depot;
    
    Node() {}
    Node(const int id, const int demand, const int draught) : id{id}, demand{demand}, draught{draught}, depot{id == 0} {}
};

#endif