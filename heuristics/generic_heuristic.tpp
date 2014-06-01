template<typename RequestPricer, typename RequestComparator, typename InsertionPricer, typename InsertionComparator>
GenericHeuristic<RequestPricer, RequestComparator, InsertionPricer, InsertionComparator>::GenericHeuristic(std::shared_ptr<const RawData> rd, const RequestPricer request_pricer, const RequestComparator request_comparator, const InsertionPricer insertion_pricer, const InsertionComparator insertion_comparator) : rd(rd), request_pricer(request_pricer), request_comparator(request_comparator), insertion_pricer(insertion_pricer), insertion_comparator(insertion_comparator) {
    unevaded_requests = std::vector<int>(0);
    unevaded_requests.reserve(rd->n);
    for(int i = 1; i <= rd->n; i++) { unevaded_requests.push_back(i); }
    std::sort(unevaded_requests.begin(), unevaded_requests.end(), request_comparator);
    std::reverse(unevaded_requests.begin(), unevaded_requests.end()); // So we can pop_back() the best request

    p = GenericPath();
    p.path.reserve(2 * rd->n + 2);
    p.load.reserve(2 * rd->n + 2);
    p.path.push_back(0); p.path.push_back(2 * rd->n + 1);
    p.load.push_back(0); p.load.push_back(0);
}