template<typename InsertionPricer, typename InsertionComparator>
GenericPath BestInsertionHeuristic<InsertionPricer, InsertionComparator>::solve() {
    while(this->unevaded_requests.size() > 0) {
        int best_cost = std::numeric_limits<int>::max();
        int best_load = 0;
        int best_request = this->unevaded_requests.at(0);
        bool managed_to_insert = false;
        GenericPath new_path;
        
        for(int i : this->unevaded_requests) {
            for(int x = 1; x < this->p.path.size(); x++) {
                for(int y = x; y < this->p.path.size(); y++) {
                    auto result = HeuristicHelper::feasible_and_improves<InsertionPricer, InsertionComparator>(i, x, y, best_cost, best_load, this->p, this->rd, this->insertion_pricer, this->insertion_comparator);
                    if(result.first) {
                        managed_to_insert = true;
                        new_path = result.second;
                        best_cost = new_path.cost;
                        best_load = new_path.total_load;
                        best_request = i;
                    }
                }
            }
        }
        
        if(managed_to_insert) {
            this->p = new_path;
            this->unevaded_requests.erase(std::remove(this->unevaded_requests.begin(), this->unevaded_requests.end(), best_request), this->unevaded_requests.end());
        } else {
            throw std::runtime_error("Can't insert any request");
        }
    }
    
    return this->p;
}