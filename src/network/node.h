#ifndef NODE_H
#define NODE_H

struct node {
    int id;
    int demand;
    int draught;
    bool depot;
    
    node() {}
    node(int id, int demand, int draught) : id{id}, demand{demand}, draught{draught}, depot{id == 0} {}
};

#endif