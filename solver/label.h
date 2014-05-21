#ifndef LABEL_H
#define LABEL_H

#include <memory>

#include <boost/utility.hpp>

#include <network/graph.h>

struct Label {
    int load;
    int started_requests;
    int open_requests;
    double cost;
    
    Label(const int load, const int started_requests, const int open_requests, const double cost) : load(load), started_requests(started_requests), open_requests(open_requests), cost(cost) {}
    
    bool operator==(const Label& other) const;
    bool operator<(const Label& other) const;
};

struct LabelExtender {
    bool operator()(const BoostGraph& g, Label& new_label, const Label& label, Edge e) const;
};

struct LabelDominator {
    bool operator()(const Label& l1, const Label& l2) const;
};

class NodeIdFunctor {
    const std::shared_ptr<Graph> graph;
    
public:
    typedef int result_type;
  
    NodeIdFunctor(const std::shared_ptr<Graph> graph) : graph(graph) {}
  
    result_type operator()(const Vertex v) const {
        return graph->g[v]->id;
    }
};

class ArcIdFunctor {
    const std::shared_ptr<Graph> graph;

public:
    typedef int result_type;
  
    ArcIdFunctor(const std::shared_ptr<Graph> graph) : graph(graph) {}
  
    result_type operator()(const Edge e) const {
        return graph->g[e]->id;
    }
};

template<typename Fun, typename Arg> class FunctionPropertyMap {
    const Fun& f;
  
public:
    typedef typename Fun::result_type value_type;
  
    explicit FunctionPropertyMap(const Fun& f) : f(f) {}
  
    friend value_type get(const FunctionPropertyMap& pm, const Arg& arg) {
        return pm.f(arg);
    }
    
    value_type operator[](const Arg& arg) const {
        return f(arg);
    }
};

namespace boost{
    template<typename Fun, typename Arg> struct property_traits<FunctionPropertyMap<Fun, Arg>> {
        typedef typename boost::result_of<Fun(Arg)>::type value_type;
        typedef value_type reference;
        typedef Arg key_type;
        typedef readable_property_map_tag category;
    };
}

template<typename Arg, typename Fun> FunctionPropertyMap<Fun, Arg> make_property_map(const Fun& f) {
    return FunctionPropertyMap<Fun, Arg>(f);
}

#endif