#pragma once

#include <beauty/route.hpp>
#include <beauty/export.hpp>

#include <boost/beast.hpp>

namespace beast = boost::beast;

namespace beauty
{
// --------------------------------------------------------------------------
class BEAUTY_EXPORT router {
public:
    using routes = std::unordered_map<beast::http::verb, std::vector<route>>;

    void add_route(beast::http::verb v, route&& r);

    routes::const_iterator find(beast::http::verb v) const noexcept {
        return _routes.find(v);
    }

    routes::const_iterator begin() const noexcept { return _routes.begin(); }
    routes::const_iterator end() const noexcept { return _routes.end(); }

private:
    routes      _routes;
};

}
