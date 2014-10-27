#ifndef NODE_H
#define NODE_H

struct Node {
    unsigned int id;
    int demand;
    int draught;
    bool depot;
    
    Node() {}
    Node(unsigned int id, int demand, int draught) : id{id}, demand{demand}, draught{draught}, depot{id == 0} {}
};

#endif