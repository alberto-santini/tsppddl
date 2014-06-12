#include <network/path.h>

Path::Path(unsigned int expected_length) {
    path.reserve(expected_length);
    load.reserve(expected_length);
    total_load = 0;
    total_cost = 0;
}