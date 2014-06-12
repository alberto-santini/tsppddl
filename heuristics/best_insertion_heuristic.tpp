template<class IC>
BestInsertionHeuristic<IC>::BestInsertionHeuristic(const std::shared_ptr<const Graph> g, const IC ic) : Heuristic{g}, insertion_comparator{ic} {}

template<class IC>
Path BestInsertionHeuristic<IC>::solve() {
    while(this->remaining_requests.size() > 0) {
        int best_cost {std::numeric_limits<int>::max()};
        int best_load {0};
        int best_request {this->remaining_requests.at(0)};
        bool managed_to_insert {false};
        Path new_path;
        
        for(int req : this->remaining_requests) {
            for(int x = 1; x < this->p.path.size(); x++) {
                for(int y = x; y < this->p.path.size(); y++) {
                    auto result = HeuristicHelper::insert<IC>(this->g, this->insertion_comparator, req, x, y, this->p, best_cost, best_load);
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
            throw std::runtime_error("Can't insert any request");
        }
    }
    
    return this->p;
}