template<class RequestComparator, class InsertionComparator, class RequestsVectorSorter>
heuristic_with_ordered_requests<RequestComparator, InsertionComparator, RequestsVectorSorter>::heuristic_with_ordered_requests(const tsp_graph& g, const RequestComparator& rc, const InsertionComparator& ic, const RequestsVectorSorter& requests_vector_sorter) : heuristic{g}, insertion_comparator(ic), requests_vector_sorter(requests_vector_sorter) {
    if(requests_vector_sorter.is_custom) {
        requests_vector_sorter.sort_in_place(remaining_requests);
    } else {
        std::sort(remaining_requests.begin(), remaining_requests.end(), rc);
        std::reverse(remaining_requests.begin(), remaining_requests.end()); // Reverse to be able to pop_back() the best
    }
}

template<class RequestComparator, class InsertionComparator, class RequestsVectorSorter>
boost::optional<path> heuristic_with_ordered_requests<RequestComparator, InsertionComparator, RequestsVectorSorter>::solve() {
    while(this->remaining_requests.size() > 0) {
        auto req = remaining_requests.back();
        if(insert(req)) {
            this->remaining_requests.pop_back();
        } else {
            return boost::none;
        }
    }
    
    return this->p;
}

template<class RequestComparator, class InsertionComparator, class RequestsVectorSorter>
bool heuristic_with_ordered_requests<RequestComparator, InsertionComparator, RequestsVectorSorter>::insert(int req) {
    auto best_cost = std::numeric_limits<int>::max();
    auto best_load = std::numeric_limits<int>::max();
    auto managed_to_insert = false;
    path new_path;
    
    for(auto x = 1u; x < this->p.path_v.size(); x++) {
        for(auto y = x; y < this->p.path_v.size(); y++) {
            auto result = heuristic_helper::insert<InsertionComparator>(this->g, this->insertion_comparator, req, x, y, this->p, best_cost, best_load);
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