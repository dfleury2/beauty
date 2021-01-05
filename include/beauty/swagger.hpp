#pragma once

#include <string>
#include <vector>

namespace beauty
{
class route;

// --------------------------------------------------------------------------
struct server_info {
    std::string title;
    std::string description;
    std::string version;
};

// --------------------------------------------------------------------------
struct route_parameter {
    std::string name;
    std::string in;             // path or query
    std::string description;
    std::string type;
    std::string format;
    bool required = false;
};

// --------------------------------------------------------------------------
struct route_info {
    std::string description;
    std::vector<route_parameter> route_parameters;
};

// --------------------------------------------------------------------------
std::string
swagger_path(const beauty::route& route);

}
