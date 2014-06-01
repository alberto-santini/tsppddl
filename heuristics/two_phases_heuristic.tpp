template<typename RequestPricer, typename RequestComparator, typename InsertionPricer, typename InsertionComparator>
bool TwoPhasesHeuristic<RequestPricer, RequestComparator, InsertionPricer, InsertionComparator>::perform_best_insertion_for_request(const int i) {
    int best_cost = std::numeric_limits<int>::max();
    int best_load = std::numeric_limits<int>::max();
    bool managed_to_insert = false;
    GenericPath new_path;
    
    for(int x = 1; x < this->p.path.size(); x++) {
        for(int y = x; y < this->p.path.size(); y++) {
            auto result = HeuristicHelper::feasible_and_improves<InsertionPricer, InsertionComparator>(i, x, y, best_cost, best_load, this->p, this->rd, this->insertion_pricer, this->insertion_comparator);
            if(result.first) {
                managed_to_insert = true;
                new_path = result.second;
                best_cost = new_path.cost;
                best_load = new_path.total_load;
            }
        }
    }
    
    if(managed_to_insert) {
        this->p = new_path;
    }
    
    return managed_to_insert;
}

template<typename RequestPricer, typename RequestComparator, typename InsertionPricer, typename InsertionComparator>
GenericPath TwoPhasesHeuristic<RequestPricer, RequestComparator, InsertionPricer, InsertionComparator>::solve() {
    while(this->unevaded_requests.size() > 0) {
        int req = this->unevaded_requests.back();
        if(perform_best_insertion_for_request(req)) {
            this->unevaded_requests.pop_back();
        } else {
            throw std::runtime_error("Found a request I can't insert anywhere!");
        }
    }
    
    return this->p;
}