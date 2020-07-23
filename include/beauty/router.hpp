#pragma once

#include <beauty/route.hpp>

#include <boost/beast.hpp>

namespace beast = boost::beast;

namespace beauty
{
// --------------------------------------------------------------------------
class router {
public:
    using routes = std::unordered_map<beast::http::verb, std::vector<route>>;

    void add_route(beast::http::verb v, route&& r);

    routes::const_iterator find(beast::http::verb v) const;

    routes::const_iterator begin() const { return _routes.begin(); }
    routes::const_iterator end() const { return _routes.end(); }

private:
    // Could be replaced by a std::array
    routes      _routes;
};

}
