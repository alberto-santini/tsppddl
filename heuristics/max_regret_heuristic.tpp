template<class IC, class RG>
Path MaxRegretHeuristic<IC, RG>::solve() {
    int rs;
    
    while((rs = this->remaining_requests.size()) > 0) {
        std::vector<double> regrets(rs, 0);
        std::vector<Path> new_possible_paths(rs, 0);
        int ps {static_cast<int>(this->p.path.size())};
                
        for(int req_id = 0; req_id < rs; req_id++) {
            // BestCost, SecondBestCost, BestLoad, SecondBestLoad
            int bc {std::numeric_limits<int>::max()}, sbc {std::numeric_limits<int>::max()}, bl {0}, sbl {0};
            bool managed_to_insert {false};
            Path new_path;
            
            for(int x = 1; x < ps; x++) {
                for(int y = x; y < ps; y++) {
                    int req {this->remaining_requests[req_id]};                    
                    auto result = HeuristicHelper::insert<IC>(this->g, this->insertion_comparator, req, x, y, this->p, sbc, sbl);
                                        
                    if(result.first) {
                        managed_to_insert = true;
                        if(this->insertion_comparator(result.second.total_cost, result.second.total_load, bc, bl)) {
                            sbc = bc; sbl = bl; new_path = result.second; bc = new_path.total_cost; bl = new_path.total_load;
                        } else {
                            sbc = result.second.total_cost; sbl = result.second.total_load;
                        }
                    }
                }
            }
            
            if(managed_to_insert) {
                regrets[req_id] = this->regret_calculator(bc, bl, sbc, sbl);
                new_possible_paths[req_id] = new_path;
            } else {
                regrets[req_id] = -1;
                new_possible_paths[req_id] = Path();
            }
        }
        
        double best_regret {regrets[0]};
        int best_regret_id {0};
        
        for(int req_id = 1; req_id < rs; req_id++) {
            if(regrets[req_id] > best_regret) {
                best_regret = regrets[req_id];
                best_regret_id = req_id;
            }
        }
        
        if(best_regret >= 0) {
            this->p = new_possible_paths[best_regret_id];
            this->remaining_requests.erase(this->remaining_requests.begin() + best_regret_id);
        } else {
            throw std::runtime_error("Can't insert any request!");
        }
    }
    
    return this->p;
}