#ifndef PORT_H
#define PORT_H

class Port {
public:
    int id;
    int draught;
    bool is_depot;
    int x;
    int y; 
    
    Port(const int id, const int draught, const bool is_depot, const int x, const int y) : id(id), draught(draught), is_depot(is_depot), x(x), y(y) {}
};

#endif