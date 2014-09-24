#include <solver/bc/callbacks/callbacks_helper.h>

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

void ch::sets_info::print_set(std::ostream& os, const bvec& set) const {
    for(int i = 1; i < set.size(); i++) { if(set[i]) { os << i << " "; } }
}

void ch::sets_info::print_S(std::ostream& os) const {
    print_set(os, in_S);
}

void ch::sets_info::print_fs(std::ostream& os) const {
    print_set(os, in_fs);
}

void ch::sets_info::print_ss(std::ostream& os) const {
    print_set(os, in_ss);
}

void ch::sets_info::print_ts(std::ostream& os) const {
    print_set(os, in_ts);
}