template<typename InsertionPricer, typename InsertionComparator, typename RegretCalculator>
GenericPath MaxRegretHeuristic<InsertionPricer, InsertionComparator, RegretCalculator>::solve() {    
    while(this->unevaded_requests.size() > 0) {
        std::vector<double> regrets(this->unevaded_requests.size(), 0);
        std::vector<GenericPath> new_possible_paths(this->unevaded_requests.size(), GenericPath());
        
        for(int r_id = 0; r_id < this->unevaded_requests.size(); r_id++) {
            int best_cost = std::numeric_limits<int>::max();
            int second_best_cost = best_cost;
            int best_load = 0;
            int second_best_load = best_load;
            
            bool managed_to_insert = false;
            GenericPath new_path;
            
            for(int x = 1; x < this->p.path.size(); x++) {
                for(int y = x; y < this->p.path.size(); y++) {
                    auto result = HeuristicHelper::feasible_and_improves<InsertionPricer, InsertionComparator>(this->unevaded_requests[r_id], x, y, second_best_cost, second_best_load, this->p, this->rd, this->insertion_pricer, this->insertion_comparator);
                    if(result.first) {
                        managed_to_insert = true;
                        if(this->insertion_comparator(result.second.cost, result.second.total_load, best_cost, best_load)) {
                            second_best_cost = best_cost;
                            second_best_load = best_load;
                            new_path = result.second;
                            best_cost = new_path.cost;
                            best_load = new_path.total_load;
                        } else {
                            second_best_cost = result.second.cost;
                            second_best_load = result.second.total_load;
                        }
                    }
                }
            }
            
            if(managed_to_insert) {
                regrets[r_id] = this->regret_calculator(best_cost, best_load, second_best_cost, second_best_load);
                new_possible_paths[r_id] = new_path;
            } else {
                regrets[r_id] = -1;
                new_possible_paths[r_id] = GenericPath();
            }
        }
        
        double best_regret = regrets[0];
        int best_regret_idx = 0;
        
        for(int r_id = 1; r_id < this->unevaded_requests.size(); r_id++) {
            if(regrets[r_id] > best_regret) {
                best_regret = regrets[r_id];
                best_regret_idx = r_id;
            }
        }
        
        if(best_regret >= 0) {
            this->p = new_possible_paths[best_regret_idx];
            this->unevaded_requests.erase(this->unevaded_requests.begin() + best_regret_idx);
        } else {
            throw std::runtime_error("I can't insert any request!");
        }
    }
    
    return this->p;
}