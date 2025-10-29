#pragma once

#include <beauty/export.hpp>

#include <string>
#include <vector>

#include <optional>
#include <boost/json.hpp>

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
struct body_schema {
    std::string schema_name; // "text/plain", "application/json", etc. check openapi documentation for all the supported schemas
    boost::json::object schema;
};

// --------------------------------------------------------------------------
struct request_body {
    bool required = false;
    std::vector<body_schema> body_schemas;
};


// --------------------------------------------------------------------------
struct route_info {
    std::string description;
    std::vector<route_parameter> route_parameters;
    std::vector<std::string> tags;
    request_body body;
};

// --------------------------------------------------------------------------
BEAUTY_EXPORT
std::string
swagger_path(const beauty::route& route);

}
