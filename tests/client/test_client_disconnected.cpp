#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/client.hpp>

using namespace std::chrono_literals;

// --------------------------------------------------------------------------
struct ClientDisconnectedFixture
{
    ClientDisconnectedFixture()
    {
        server.concurrency(2); // 2 threads are needed here because of synchronous get
        server.get("/index.html", [](const beauty::request& req, beauty::response& res) {
            res.set(beauty::content_type::text_plain);
            res.body() = "GET VERB";
        });

        server.listen();
        port = server.port();
    }

    ~ClientDisconnectedFixture() {
        server.stop();
    }

    beauty::server server;
    int port = 0;
};

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientDisconnectedFixture, "Get asynchronous disconnected")
{
    auto url = "http://127.0.0.1:" + std::to_string(port) + "/index.html";

    beauty::client client;

    bool called = false;
    beauty::request req;
    req.method(beast::http::verb::get);
    req.keep_alive(false); // force to close the connection

    client.send_request(std::move(req), std::chrono::milliseconds(0), url,
            [&called](boost::system::error_code ec, beauty::response&& response) {
                CHECK_EQ(ec, boost::system::errc::success);
                CHECK_EQ(response.body(), "GET VERB");
                called = true;
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(called);

    called = false;
    client.get(
            url,
            [&called](boost::system::error_code ec, beauty::response&& response) {
                CHECK_EQ(ec, boost::system::errc::success);
                CHECK_EQ(response.body(), "GET VERB");
                called = true;

            });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(called);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientDisconnectedFixture, "Get asynchronous rearm")
{
    auto url = "http://127.0.0.1:" + std::to_string(port) + "/index.html";

    beauty::client client;

    bool called = false;

    client.get(url,
            [&called](boost::system::error_code ec, beauty::response&& response) {
                CHECK_EQ(ec, boost::system::errc::success);
                CHECK_EQ(response.body(), "GET VERB");
                called = true;
            });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(called);

    called = false;
    client.get(url,
            [&called](boost::system::error_code ec, beauty::response&& response) {
                CHECK_EQ(ec, boost::system::errc::success);
                CHECK_EQ(response.body(), "GET VERB");
                called = true;
            });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(called);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientDisconnectedFixture, "Get asynchronous no rearm")
{
    auto url = "http://127.0.0.1:" + std::to_string(port) + "/index.html";

    beauty::client client;

    int called = 0;

    client.get(url,
               [&called](boost::system::error_code ec, beauty::response&& response) {
                   CHECK_EQ(ec, boost::system::errc::success);
                   CHECK_EQ(response.body(), "GET VERB");
                   ++called;
               });

    client.get(url,
               [&called](boost::system::error_code ec, beauty::response&& response) {
                   CHECK_EQ(ec, boost::system::errc::success);
                   CHECK_EQ(response.body(), "GET VERB");
                   ++called;
               });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_EQ(called, 2);
}
