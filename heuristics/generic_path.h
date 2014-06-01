#ifndef GENERIC_PATH_H
#define GENERIC_PATH_H

class GenericPath {
public:
    std::vector<int> path;
    std::vector<int> load;
    int total_load;
    int cost;
    
    GenericPath() : path(std::vector<int>(0)), load(std::vector<int>(0)), total_load(0), cost(0) {}
    GenericPath(const std::vector<int> ps, const std::vector<int> l, const int tl, const int c) : path(ps), load(l), total_load(tl), cost(c) {}
};

#endif