#include <program/program.h>

#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    auto args = std::vector<std::string>(argv + 1, argv + argc);
    
    std::srand(123321); // For reproducibility
    
    auto p = program(args);
    
    return 0;
}