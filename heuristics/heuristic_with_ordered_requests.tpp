template<class RC, class IC>
heuristic_with_ordered_requests<RC, IC>::heuristic_with_ordered_requests(const tsp_graph& g, const RC& rc, const IC& ic) : heuristic{g}, insertion_comparator(ic) {
    std::sort(remaining_requests.begin(), remaining_requests.end(), rc);
    std::reverse(remaining_requests.begin(), remaining_requests.end()); // Reverse to be able to pop_back() the best
}

template<class RC, class IC>
path heuristic_with_ordered_requests<RC, IC>::solve() {
    using namespace std::chrono;
    
    auto t_start = high_resolution_clock::now();
    
    while(this->remaining_requests.size() > 0) {
        auto req = remaining_requests.back();
        if(insert(req)) {
            this->remaining_requests.pop_back();
        } else {
            throw std::runtime_error("Can't insert the request anywhere!");
        }
    }
    
    auto t_end = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t_end - t_start);
    global::g_total_time_spent_by_heuristics += time_span.count();
    
    return this->p;
}

template<class RC, class IC>
bool heuristic_with_ordered_requests<RC, IC>::insert(int req) {
    auto best_cost = std::numeric_limits<int>::max();
    auto best_load = std::numeric_limits<int>::max();
    auto managed_to_insert = false;
    path new_path;
    
    for(auto x = 1u; x < this->p.path_v.size(); x++) {
        for(auto y = x; y < this->p.path_v.size(); y++) {
            auto result = heuristic_helper::insert<IC>(this->g, this->insertion_comparator, req, x, y, this->p, best_cost, best_load);
            if(result.first) {
                managed_to_insert = true;
                new_path = result.second;
                best_cost = new_path.total_cost;
                best_load = new_path.total_load;
            }
        }
    }
    
    if(managed_to_insert) {
        this->p = new_path;
    }
    
    return managed_to_insert;
}