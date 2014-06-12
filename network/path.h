#ifndef PATH_H
#define PATH_H

#include <vector>

class Path {
public:
    std::vector<int> path;
    std::vector<int> load;
    int total_load;
    int total_cost;
    
    Path() {}
    Path(unsigned int expected_length);
};

#endif