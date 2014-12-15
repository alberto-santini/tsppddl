template<class IC>
best_insertion_heuristic<IC>::best_insertion_heuristic(const tsp_graph& g, const IC& ic) : heuristic{g}, insertion_comparator(ic) {}

template<class IC>
path best_insertion_heuristic<IC>::solve() {
    while(this->remaining_requests.size() > 0) {
        auto best_cost = std::numeric_limits<int>::max();
        auto best_load = 0;
        auto best_request = this->remaining_requests.at(0);
        auto managed_to_insert = false;
        path new_path;
        
        for(auto req : this->remaining_requests) {
            for(auto x = 1u; x < this->p.path_v.size(); x++) {
                for(auto y = x; y < this->p.path_v.size(); y++) {
                    auto result = heuristic_helper::insert<IC>(this->g, this->insertion_comparator, req, x, y, this->p, best_cost, best_load);
                    if(result.first) {
                        managed_to_insert = true;
                        new_path = result.second;
                        best_cost = new_path.total_cost;
                        best_load = new_path.total_load;
                        best_request = req;
                    }
                }
            }
        }
        
        if(managed_to_insert) {
            this->p = new_path;
            this->remaining_requests.erase(std::remove(this->remaining_requests.begin(), this->remaining_requests.end(), best_request), this->remaining_requests.end());
        } else {
            throw std::runtime_error("Can't insert any request! (best insertion)");
        }
    }

    return this->p;
}