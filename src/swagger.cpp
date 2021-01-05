#include <beauty/swagger.hpp>

#include <beauty/route.hpp>

namespace beauty
{

// --------------------------------------------------------------------------
// Create a path swagger compatible
// --------------------------------------------------------------------------
std::string
swagger_path(const beauty::route& route)
{
    std::string path;

    auto segments = route.segments();
    int index = 0;
    for(auto& s : segments) {
        if (s[0] == ':') s = "{" + s.substr(1) + "}";

        path += (index ? "/" : "") + s;

        ++index;
    }
    return path;
}

}
