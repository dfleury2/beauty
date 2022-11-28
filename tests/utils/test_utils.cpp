#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/utils.hpp>
#include <beauty/url.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Escape")
{
    std::string s("some text with % chars to escape Ã Ã©iÃ´u");

    CHECK_EQ(beauty::unescape(beauty::escape(s)), s);

    CHECK_EQ(beauty::escape("must be"), "must%20be");
}

// --------------------------------------------------------------------------
TEST_CASE("Unescape")
{
    CHECK_EQ(beauty::unescape("/tmp/srv"), "/tmp/srv");
    CHECK_EQ(beauty::unescape("%2ftmp%2fsrv"), "/tmp/srv");
    CHECK_EQ(beauty::unescape("some spaces in a%20line"), "some spaces in a line");

    CHECK_EQ(beauty::unescape("%252ftmp%252fsrv"), "%2ftmp%2fsrv");
}
