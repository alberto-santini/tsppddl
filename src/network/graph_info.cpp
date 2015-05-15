#include <network/graph_info.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <sstream>

graph_info::graph_info(int n, int capacity, std::string instance_path) : n{n}, capacity{capacity}, instance_path{instance_path} {
    auto path_parts = std::vector<std::string>();
    boost::split(path_parts, instance_path, boost::is_any_of("/"));
    
    if(path_parts.size() > 1) {
        instance_dir = path_parts.at(path_parts.size() - 2);
    }
    
    auto file_parts = std::vector<std::string>();
    boost::split(file_parts, path_parts.back(), boost::is_any_of("."));
    
    if(file_parts.size() > 1) {
        file_parts.pop_back();
    }
    
    instance_name = boost::algorithm::join(file_parts, ".");
    
    auto name_parts = std::vector<std::string>();
    boost::split(name_parts, instance_name, boost::is_any_of("_"));
    std::stringstream ss;

    instance_base_name = name_parts[0];
    
    if(DEBUG) {
        #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        ss << name_parts[1];
        int ln = 0;
        ss >> ln;
        assert(ln == n);
        #pragma GCC diagnostic pop
    }
    
    ss.str(std::string());
    ss.clear();
    ss << name_parts[2];
    ss >> h;
    
    ss.str(std::string());
    ss.clear();
    ss << name_parts[3];    
    ss >> k;
}