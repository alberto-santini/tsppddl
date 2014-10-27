#ifndef ARC_H
#define ARC_H

struct Arc {
    int id;
    int cost;
    
    Arc() {}
    Arc(int id, int cost) : id{id}, cost{cost} {}
};

#endif