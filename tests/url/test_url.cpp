#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/url.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Http or https")
{
    CHECK(beauty::url("http://localhost.com").is_http());
    CHECK_FALSE(beauty::url("http://localhost.com").is_https());

    CHECK_FALSE(beauty::url("https://localhost.com").is_http());
    CHECK(beauty::url("https://localhost.com").is_https());
}

// --------------------------------------------------------------------------
TEST_CASE("Host and user info")
{
    SUBCASE("No port") {
        CHECK_EQ(beauty::url("http://localhost.com").host(), "localhost.com");
        CHECK_EQ(beauty::url("http://dfleury@localhost.com").host(), "localhost.com");
        CHECK_EQ(beauty::url("http://dfleury:nopass@localhost.com").host(), "localhost.com");

        CHECK_EQ(beauty::url("http://localhost.com").login(), "");
        CHECK_EQ(beauty::url("http://dfleury@localhost.com").login(), "dfleury");
        CHECK_EQ(beauty::url("http://dfleury:nopass@localhost.com").login(), "dfleury");

        CHECK_EQ(beauty::url("http://localhost.com").password(), "");
        CHECK_EQ(beauty::url("http://dfleury@localhost.com").password(), "");
        CHECK_EQ(beauty::url("http://dfleury:nopass@localhost.com").password(), "nopass");
    }

    SUBCASE("With port") {
        CHECK_EQ(beauty::url("http://localhost.com").port(), 0);
        CHECK_EQ(beauty::url("http://localhost.com:8085").port(), 8085);
        CHECK_EQ(beauty::url("http://dfleury@localhost.com:8085").port(), 8085);
        CHECK_EQ(beauty::url("http://dfleury:nopass@localhost.com:8085").port(), 8085);
    }

    SUBCASE("Ipv6") {
        CHECK_EQ(beauty::url("http://[::1]").host(), "::1");
        CHECK_EQ(beauty::url("http://[::1]:8085").host(), "::1");
        CHECK_EQ(beauty::url("http://[::1]:8085").port(), 8085);
        CHECK_EQ(beauty::url("http://dfleury@[::1]:8085").host(), "::1");
        CHECK_EQ(beauty::url("http://dfleury@[::1]:8085").port(), 8085);
        CHECK_EQ(beauty::url("http://dfleury:nopass@[::1]:8085").port(), 8085);
        CHECK_EQ(beauty::url("ws://[::1]:8085/chat/MyRoom").port(), 8085);
    }
}

// --------------------------------------------------------------------------
TEST_CASE("Path and Query")
{
    SUBCASE("No query") {
        CHECK_EQ(beauty::url("https://localhost.com").path(), "/");
        CHECK_EQ(beauty::url("https://localhost.com/").path(), "/");
        CHECK_EQ(beauty::url("http://127.0.0.1/path").path(), "/path");
        CHECK_EQ(beauty::url("https://127.0.0.1/long/path").path(), "/long/path");
        CHECK_EQ(beauty::url("http://localhost.com/long/path/").path(), "/long/path/");
    }

    SUBCASE("With query") {
        CHECK_EQ(beauty::url("https://localhost.com/").query(), "");
        CHECK_EQ(beauty::url("http://127.0.0.1/?query=yes").query(), "?query=yes");
        CHECK_EQ(beauty::url("http://localhost.com/path?query=yes").query(), "?query=yes");
        CHECK_EQ(beauty::url("https://127.0.0.1/long/path?query=yes").query(), "?query=yes");
        CHECK_EQ(beauty::url("http://localhost.com/long/path/?query=yes").query(), "?query=yes");
    }
}

// --------------------------------------------------------------------------
TEST_CASE("Tcp url")
{
    SUBCASE("No connection id") {
        auto url = beauty::url{"tcp://127.0.0.1:4000/output/XDR-E/1"};
        CHECK_EQ(url.scheme(), "tcp");
        CHECK_EQ(url.login(), "");
        CHECK_EQ(url.host(), "127.0.0.1");
        CHECK_EQ(url.port(), 4000);
        CHECK_EQ(url.path(), "/output/XDR-E/1");
    }

    SUBCASE("No connection id, no port") {
        auto url = beauty::url{"tcp://127.0.0.1/output/XDR-E/1"};
        CHECK_EQ(url.scheme(), "tcp");
        CHECK_EQ(url.login(), "");
        CHECK_EQ(url.host(), "127.0.0.1");
        CHECK_EQ(url.port(), 0);
        CHECK_EQ(url.path(), "/output/XDR-E/1");

        CHECK_EQ(url.strip_login_password(), "tcp://127.0.0.1/output/XDR-E/1");
    }

    SUBCASE("With loing") {
        auto url = beauty::url{"tcp://731747d5e64147da91fad6e9b8062668@127.0.0.1:4000/output/XDR-E/1"};
        CHECK_EQ(url.scheme(), "tcp");
        CHECK_EQ(url.login(), "731747d5e64147da91fad6e9b8062668");
        CHECK_EQ(url.host(), "127.0.0.1");
        CHECK_EQ(url.port(), 4000);

        CHECK_EQ(url.strip_login_password(), "tcp://127.0.0.1:4000/output/XDR-E/1");
    }

    SUBCASE("With login") {
        auto url = beauty::url{"tcp://login:password@127.0.0.1:4000/output/XDR-E/1"};
        CHECK_EQ(url.scheme(), "tcp");
        CHECK_EQ(url.login(), "login");
        CHECK_EQ(url.password(), "password");
        CHECK_EQ(url.host(), "127.0.0.1");
        CHECK_EQ(url.port(), 4000);

        CHECK_EQ(url.strip_login_password(), "tcp://127.0.0.1:4000/output/XDR-E/1");
    }
}


// --------------------------------------------------------------------------
TEST_CASE("Some invalid formats")
{
    CHECK_THROWS(beauty::url(""));
    CHECK_THROWS(beauty::url("http:/"));
    CHECK_THROWS(beauty::url("http://"));
    CHECK_THROWS(beauty::url("http:/dfleury@host@.com"));
    CHECK_THROWS(beauty::url("http:/ex:pan:dst@host.com"));
    CHECK_THROWS(beauty::url("http:/expan:dst@host.com:8085:85"));
    CHECK_THROWS(beauty::url("http:/expan:dst@host.com:port"));
    CHECK_THROWS(beauty::url("http://[::1]:").port(), 0);
}
