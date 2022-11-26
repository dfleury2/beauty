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
TEST_CASE("Start and stop - external context")
{
    boost::asio::io_context ioc;
    beauty::application app(ioc);
    beauty::server server(app);

    std::thread([&ioc] { ioc.run(); }).detach();

    app.start();

    server.listen();

    CHECK_NE(server.endpoint().port(), 0);

    app.stop();
    ioc.stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Start and stop with acceptor failure - external context")
{
    boost::asio::io_context ioc;
    beauty::application app(ioc);

    // two servers that will listen on the same port
    beauty::server server1(app);
    beauty::server server2(app);

    std::thread([&ioc] { ioc.run(); }).detach();

    app.start();

    server1.listen();

    CHECK_NE(server1.endpoint().port(), 0);

    server2.listen(server1.endpoint().port());

    // This test was added to test an infinite loop in acceptor. Currently there's
    // no way easy to check if the loop exists or not except by looking into
    // stderr of the test. In the future an explicit error callback could be added,
    // in which case errors could be intercepted and this test could check if error
    // count is not very large.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    app.stop();
    ioc.stop();
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
