template<class RC, class IC>
HeuristicWithOrderedRequests<RC, IC>::HeuristicWithOrderedRequests(const std::shared_ptr<const Graph> g, const RC rc, const IC ic) : Heuristic{g}, insertion_comparator{ic} {
    std::sort(remaining_requests.begin(), remaining_requests.end(), rc);
    std::reverse(remaining_requests.begin(), remaining_requests.end()); // Reverse to be able to pop_back() the best
}

template<class RC, class IC>
Path HeuristicWithOrderedRequests<RC, IC>::solve() {
    while(this->remaining_requests.size() > 0) {
        int req {remaining_requests.back()};
        if(insert(req)) {
            this->remaining_requests.pop_back();
        } else {
            throw std::runtime_error("Can't insert the request anywhere!");
        }
    }
    
    return this->p;
}

template<class RC, class IC>
bool HeuristicWithOrderedRequests<RC, IC>::insert(const int req) {
    int best_cost {std::numeric_limits<int>::max()};
    int best_load {std::numeric_limits<int>::max()};
    bool managed_to_insert {false};
    Path new_path;
    
    for(int x = 1; x < this->p.path.size(); x++) {
        for(int y = x; y < this->p.path.size(); y++) {
            auto result = HeuristicHelper::insert<IC>(this->g, this->insertion_comparator, req, x, y, this->p, best_cost, best_load);
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