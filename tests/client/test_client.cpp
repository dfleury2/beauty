#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/client.hpp>

#include <chrono>

using namespace std::chrono_literals;

namespace {
bool call = false;
}

// --------------------------------------------------------------------------
struct ClientFixture
{
    ClientFixture()
    {
        server.concurrency(2); // 2 threads are needed here because of synchronous get
        server.get("/index.html", [](const beauty::request& req, beauty::response& res) {
            auto delay = req.a("delay").as_double();
            auto filename = req.a("filename").as_string();

            res.set(beauty::content_type::text_plain);
            res.body() = "GET VERB";
            if (!filename.empty()) res.body() += ":" + filename;
            call = true;

            if (delay != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        });

        server.del("/index.html", [](const beauty::request& req, beauty::response& res) {
            auto delay = req.a("delay").as_double();

            res.set(beauty::content_type::text_plain);
            res.body() = "DELETE VERB";
            call = true;

            if (delay != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        });

        server.post("/index.html", [](const beauty::request& req, beauty::response& res) {
            auto delay = req.a("delay").as_double();
            auto with_body = req.a("with_body");

            res.set(beauty::content_type::text_plain);
            res.body() = "POST VERB - BODY SIZE=" + std::to_string(req.body().size());
            if (with_body == "true") res.body() += ":" + req.body();
            call = true;

            if (delay != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        });

        server.put("/index.html", [](const beauty::request& req, beauty::response& res) {
            auto delay = req.a("delay").as_double();

            res.set(beauty::content_type::text_plain);
            res.body() = "PUT VERB - BODY SIZE=" + std::to_string(req.body().size());
            call = true;

            if (delay != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        });
        server.listen();
        port = server.port();
    }

    ~ClientFixture() {
        server.stop();
    }

    beauty::server server;
    int port;
};

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Synchronous")
{
    call = false;
    auto[ec, response] = beauty::client().get("http://127.0.0.1:" + std::to_string(port) + "/index.html");

    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "GET VERB");
    CHECK(call);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Synchronous with escape/unescape")
{
    SUBCASE("No escape") {
        auto[ec, response] = beauty::client().get("http://127.0.0.1:" + std::to_string(port) + "/index.html?filename=/tmp/srv/data.pcapng");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "GET VERB:/tmp/srv/data.pcapng");
    }

    SUBCASE("No escape") {
        auto[ec, response] = beauty::client().get("http://127.0.0.1:" + std::to_string(port) + "/index.html?filename=%2ftmp%2fsrv%2fdata%2Epcapng");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "GET VERB:/tmp/srv/data.pcapng");
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Asynchronous")
{
    bool called = false;

    auto cb = [&called](boost::system::error_code ec, beauty::response&& response) {
        called = true;
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "GET VERB");
    };

    beauty::client().get("http://127.0.0.1:" + std::to_string(port) + "/index.html", cb);

    std::this_thread::sleep_for(100ms);

    CHECK(called);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Synchronous Timeout ")
{
    beauty::client client;
    SUBCASE("No timeout") {
        auto[ec, response] = client.get_before(500ms, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.250");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "GET VERB");
    }

    SUBCASE("Timeout") {
        auto[ec, response] = client.get_before(250ms, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.500");
        CHECK_EQ(ec, boost::system::errc::timed_out);
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Asynchronous Timeout ")
{
    bool called = false;

    beauty::client client;

    SUBCASE("Get No timeout") {
        called = false;

        auto cb = [&called](boost::system::error_code ec, beauty::response&& response) {
            called = true;
            CHECK_EQ(ec, boost::system::errc::success);
            CHECK_EQ(response.body(), "GET VERB");
        };

        client.get_before(504ms, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.252", cb);

        std::this_thread::sleep_for(650ms);
        CHECK(called);
    }


    SUBCASE("Get Timeout") {
        called = false;

        auto cb = [&called](boost::system::error_code ec, beauty::response&& response) {
            called = !called;
            CHECK_EQ(ec, boost::system::errc::timed_out);
            CHECK_EQ(response.body().size(), 0);
        };

        client.get_before(300ms, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.500", std::move(cb));

        std::this_thread::sleep_for(400ms);
        CHECK(called);

        std::this_thread::sleep_for(200ms);
        CHECK(called);
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Delete Synchronous")
{
    call = false;
    auto[ec, response] = beauty::client().del("http://127.0.0.1:" + std::to_string(port) + "/index.html");

    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "DELETE VERB");
    CHECK(call);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Post Synchronous")
{
    call = false;
    auto[ec, response] = beauty::client().post("http://127.0.0.1:" + std::to_string(port) + "/index.html", "I am the request body");

    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "POST VERB - BODY SIZE=21");
    CHECK(call);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Post Synchronous with escape/unescape")
{
    SUBCASE("No escape") {
        auto[ec, response] = beauty::client().post("http://127.0.0.1:" + std::to_string(port) + "/index.html?with_body=true",
                                                   "I am the request body");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "POST VERB - BODY SIZE=21:I am the request body");
    }

    SUBCASE("No escape with special char") {
        auto[ec, response] = beauty::client().post("http://127.0.0.1:" + std::to_string(port) + "/index.html?with_body=true",
                                                   "I%20am%20the%20request%20body");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "POST VERB - BODY SIZE=29:I%20am%20the%20request%20body");
    }

    SUBCASE("No escape with special char") {
        auto[ec, response] = beauty::client().post("http://127.0.0.1:" + std::to_string(port) + "/index.html?with_body=true",
                                                   beauty::escape("I am the request body"));
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "POST VERB - BODY SIZE=29:I%20am%20the%20request%20body");
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Post synchronous Timeout ")
{
    beauty::client client;
    SUBCASE("No timeout") {
        auto[ec, response] = client.post_before(0.500, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.250", "I am the POST request body");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "POST VERB - BODY SIZE=26");
    }

    SUBCASE("Timeout") {
        auto[ec, response] = client.post_before(250ms, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.500", "I am the POST request body");
        CHECK_EQ(ec, boost::system::errc::timed_out);
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Put Synchronous")
{
    call = false;
    auto[ec, response] = beauty::client().put("http://127.0.0.1:" + std::to_string(port) + "/index.html", "I am the PUT request body");

    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "PUT VERB - BODY SIZE=25");
    CHECK(call);
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Put synchronous Timeout ")
{
    beauty::client client;
    SUBCASE("No timeout") {
        auto[ec, response] = client.put_before(0.500, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.250", "I am the PUT request body");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "PUT VERB - BODY SIZE=25");
    }

    SUBCASE("Timeout") {
        auto[ec, response] = client.put_before(0.250, "http://127.0.0.1:" + std::to_string(port) + "/index.html?delay=0.500", "I am the PUT request body");
        CHECK_EQ(ec, boost::system::errc::timed_out);
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(ClientFixture, "Get Asynchronous connection failure with tiemout")
{
    int number_if_calls = 0;

    auto cb = [&number_if_calls](boost::system::error_code ec, beauty::response&& response) {
        ++number_if_calls;
    };

    beauty::client().get_before(0.250, "http://127.0.0.1:65439/index.html", cb);

    std::this_thread::sleep_for(500ms);

    CHECK_EQ(number_if_calls, 1);
}

// --------------------------------------------------------------------------
TEST_CASE("Distinct app for two servers")
{
    beauty::application app;
    beauty::server subserver(app);
    subserver.get("/subquery", [](const beauty::request& req, beauty::response& res) {
        res.body() = "OK";
    });
    subserver.listen();
    int sub_port = subserver.port();

    beauty::server server;
    server.get("/query", [sub_port](const beauty::request& req, beauty::response& res) {
        res.body() = "OK";

        // This synchronous client will lock the io context for the subquery if no distinct app is used
        beauty::client client;
        auto [ec, response] = client.get("http://127.0.0.1:" + std::to_string(sub_port) + "/subquery");
        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "OK");
    });
    server.listen();
    int port = server.port();

    beauty::client client;
    auto [ec, response] = client.get("http://127.0.0.1:" + std::to_string(port) + "/query");
    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "OK");
}
