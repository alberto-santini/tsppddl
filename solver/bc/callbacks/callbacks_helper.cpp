#include <solver/bc/callbacks/callbacks_helper.h>

#include <sstream>

bool ch::sets_info::is_in_S(int i) const {
    if(i == 0 || i == 2*n + 1) {
        return false;
    } else {
        return in_S[i];
    }
}

bool ch::sets_info::is_in_fs(int i) const {
    if(i == 0 || i == 2*n + 1) {
        return false;
    } else {
        return in_fs[i];
    }
}

bool ch::sets_info::is_in_ss(int i) const {
    if(i == 0 || i == 2*n + 1) {
        return false;
    } else {
        return in_ss[i];
    }
}

bool ch::sets_info::is_in_ts(int i) const {
    if(i == 0 || i == 2*n + 1) {
        return false;
    } else {
        return in_ts[i];
    }
}

bool ch::sets_info::empty_S() const {
    return (boost::find(in_S, true) == boost::end(in_S));
}

int ch::sets_info::first_non_tabu() const {
    return (boost::find(in_tabu, false) - boost::begin(in_tabu));
}

std::string ch::sets_info::print_set(const bvec& set) const {
    std::stringstream ss;
    for(int i = 1; i < set.size(); i++) { if(set[i]) { ss << i << " "; } }
    return ss.str();
}

std::string ch::sets_info::print_S() const {
    print_set(in_S);
}

std::string ch::sets_info::print_fs() const {
    print_set(in_fs);
}

std::string ch::sets_info::print_ss() const {
    print_set(in_ss);
}

std::string ch::sets_info::print_ts() const {
    print_set(in_ts);
}