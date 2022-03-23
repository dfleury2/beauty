#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/router.hpp>

namespace {
auto route_matcher = [](const beauty::router& router, beauty::request& req) {
    for (const auto&[_, routes]: router) {
        for (const auto& r: routes) {
            if (r.match(req)) {
                beauty::response res;
                r.execute(req, res);
            }
        }
    }
};
}

// --------------------------------------------------------------------------
TEST_CASE("Route priority")
{
    beauty::router router;
    std::vector<int> matched_routes;

    auto cb = [&matched_routes](int idx) {
        return [&matched_routes, idx](const beauty::request&, beauty::response&) {
            matched_routes.push_back(idx);
        };
    };

    beauty::request req;

    SUBCASE("One level path") {
        router.add_route(beast::http::verb::get, beauty::route("/:A", cb(2)));
        router.add_route(beast::http::verb::get, beauty::route("/", cb(0)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa", cb(1)));

        SUBCASE("Empty") {
            matched_routes.clear();
            req.target("/");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 0);
        }

        SUBCASE("Specific path") {
            matched_routes.clear();
            req.target("/aaa");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 1);
        }

        SUBCASE("Variable path") {
            matched_routes.clear();
            req.target("/bbb");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 2);
        }
    }

    SUBCASE("Two level path") {
        router.add_route(beast::http::verb::get, beauty::route("/:A", cb(2)));
        router.add_route(beast::http::verb::get, beauty::route("/:A/:B", cb(6)));
        router.add_route(beast::http::verb::get, beauty::route("/:A/bbb", cb(5)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa", cb(1)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/:B", cb(4)));
        router.add_route(beast::http::verb::get, beauty::route("/", cb(0)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/bbb", cb(3)));

        SUBCASE("Check all ") {
            matched_routes.clear();
            req.target("/");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 0);

            matched_routes.clear();
            req.target("/any");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 2);

            matched_routes.clear();
            req.target("/any/other");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 6);

            matched_routes.clear();
            req.target("/any/bbb");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 5);

            matched_routes.clear();
            req.target("/aaa");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 1);

            matched_routes.clear();
            req.target("/aaa/other");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 4);

            matched_routes.clear();
            req.target("/");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 0);

            matched_routes.clear();
            req.target("/aaa/bbb");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 3);
        }
    }

    SUBCASE("Three level path") {
        router.add_route(beast::http::verb::get, beauty::route("/:A/:B/:C", cb(7)));
        router.add_route(beast::http::verb::get, beauty::route("/:A/:B/ccc", cb(6)));
        router.add_route(beast::http::verb::get, beauty::route("/:A/bbb/ccc", cb(4)));
        router.add_route(beast::http::verb::get, beauty::route("/:A/bbb/:C", cb(5)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/bbb/:C", cb(1)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/:B/:C", cb(3)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/:B/ccc", cb(2)));
        router.add_route(beast::http::verb::get, beauty::route("/aaa/bbb/ccc", cb(0)));

        SUBCASE("Check all ") {
            matched_routes.clear();
            req.target("/aaa/bbb/ccc");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 0);

            matched_routes.clear();
            req.target("/aaa/bbb/thing");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 1);

            matched_routes.clear();
            req.target("/aaa/other/ccc");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 2);

            matched_routes.clear();
            req.target("/aaa/other/thing");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 3);

            matched_routes.clear();
            req.target("/any/bbb/ccc");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 4);

            matched_routes.clear();
            req.target("/any/bbb/thing");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 5);

            matched_routes.clear();
            req.target("/any/other/ccc");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 6);

            matched_routes.clear();
            req.target("/any/other/thing");
            route_matcher(router, req);
            CHECK_EQ(matched_routes[0], 7);
        }
    }
}
