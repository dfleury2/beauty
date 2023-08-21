#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/version.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Start and stop")
{
    beauty::server server;

    server.listen();

    CHECK_NE(server.endpoint().port(), 0);

    server.stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Start and stop - multiple threads")
{
    beauty::server server;

    server
        .concurrency(5)
        .listen();

    CHECK_NE(server.endpoint().port(), 0);

    server.stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Server swagger info")
{
    beauty::server_info info;
    info.title = "My start and stop test server";
    info.description = "This is a test for starting and stopping a Beauty server";
    info.version = BEAUTY_PROJECT_VERSION;

    beauty::server server;
    server.info(info);

    CHECK_EQ(server.info().title, "My start and stop test server");
}
