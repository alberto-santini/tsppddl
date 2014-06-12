#ifndef ARC_H
#define ARC_H

struct Arc {
    int id;
    int cost;
    
    Arc() {}
    Arc(const int id, const int cost) : id{id}, cost{cost} {}
};

#endif