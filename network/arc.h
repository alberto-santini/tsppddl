#ifndef ARC_H
#define ARC_H

class Arc {
public:
    int id;
    double cost;
    
    Arc(const int id, const double cost) : id(id), cost(cost) {}
};

#endif