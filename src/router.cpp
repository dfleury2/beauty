#include <beauty/router.hpp>

using namespace beast::http;

namespace beauty
{
// --------------------------------------------------------------------------
void
router::add_route(beast::http::verb v, beauty::route&& r)
{
    _routes[v].push_back(std::move(r));
}

// --------------------------------------------------------------------------
router::routes::const_iterator
router::find(beast::http::verb v) const
{
    return _routes.find(v);
}

}
