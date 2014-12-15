#ifndef ARC_H
#define ARC_H

struct arc {
    int id;
    int cost;
    
    arc() {}
    arc(int id, int cost) : id{id}, cost{cost} {}
};

#endif