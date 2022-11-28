#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/route.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Empty static route")
{
    beauty::route route("/");

    beauty::request req;

    SUBCASE("/") {
        req.target("/");
        CHECK(route.match(req));
    }

    SUBCASE("/not_match") {
        req.target("/not_match");
        CHECK_FALSE(route.match(req));
    }

    SUBCASE("/not_match/neither") {
        req.target("/not_match/neither");
        CHECK_FALSE(route.match(req));
    }

    SUBCASE("/:no_no") {
        req.target("/:no_no");
        CHECK_FALSE(route.match(req));
    }
}

// --------------------------------------------------------------------------
TEST_CASE("One length static route")
{
    beauty::route route("/there");

    beauty::request req;

    SUBCASE("/there") {
        req.target("/there");
        CHECK(route.match(req));
    }

    SUBCASE("/") {
        req.target("/");
        CHECK_FALSE(route.match(req));
    }

    SUBCASE("/not_match/neither") {
        req.target("/not_match/neither");
        CHECK_FALSE(route.match(req));
    }

    SUBCASE("/:no_no") {
        req.target("/:no_no");
        CHECK_FALSE(route.match(req));
    }
}

// --------------------------------------------------------------------------
TEST_CASE("Three length static route")
{
    beauty::route route("/there/and/everywhere");

    beauty::request req;

    SUBCASE("/there/and/everywhere") {
        req.target("/there/and/everywhere");
        CHECK(route.match(req));
    }

    SUBCASE("/") {
        req.target("/");
        CHECK_FALSE(route.match(req));
    }

    SUBCASE("/there/and/everywhere/else") {
        req.target("/there/and/everywhere/else");
        CHECK_FALSE(route.match(req));
    }
}

// --------------------------------------------------------------------------
TEST_CASE("One length dynamic route")
{
    beauty::route route("/:there_id");

    beauty::request req;

    SUBCASE("/there") {
        std::string id;
        req.target("/there");
        CHECK(route.match(req));
        CHECK_EQ(req.a("there_id"), "there");
    }

    SUBCASE("/not/there") {
        req.target("/not/there");
        CHECK_FALSE(route.match(req));
    }

}

// --------------------------------------------------------------------------
TEST_CASE("Three length dynamic route")
{
    beauty::route route("/is/:there_id/anybody");

    beauty::request req;

    SUBCASE("/is/there/anybody") {
        std::string id;
        req.target("/is/there/anybody");
        CHECK(route.match(req));
        CHECK_EQ(req.a("there_id"), "there");
    }

    SUBCASE("/is/there/anybody/out") {
        req.target("/is/there/anybody/out");
        CHECK_FALSE(route.match(req));
    }
}
