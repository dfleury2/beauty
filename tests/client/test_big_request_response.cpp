#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/client.hpp>

#include <iostream>

// --------------------------------------------------------------------------
constexpr size_t operator"" _O(unsigned long long int s) { return s; }
constexpr size_t operator"" _Ko(unsigned long long int s) { return s * 1024; }
constexpr size_t operator"" _Mo(unsigned long long int s) { return s * 1024 * 1024; }

// --------------------------------------------------------------------------
std::vector sizes{ 16_O,
    1_Ko, 8_Ko, 512_Ko,
    1_Mo, 8_Mo, 16_Mo, 32_Mo,
//    128_Mo, 256_Mo, 512_Mo
};

// --------------------------------------------------------------------------
struct BigRequestResponseFixture
{
    BigRequestResponseFixture()
    {
        server.concurrency(2);  // 2 threads are needed here because of synchronous get
        server.post("/query", [](const beauty::request& req, beauty::response& res) {
            auto size = req.a("size").as_integer();

            res.set(beauty::content_type::text_plain);

            if (size == 0) {
                res.body() = "POST VERB - REQUEST BODY SIZE=" + std::to_string(req.body().size());
            } else {
                res.body() = std::string(size, '0');
            }
        });

        server.listen();
        ep = server.endpoint();
    }

    ~BigRequestResponseFixture() {
        server.stop();
    }

    beauty::server server;
    beauty::endpoint ep;
};

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(BigRequestResponseFixture, "Simple post")
{
    std::string url = "http://127.0.0.1:" + std::to_string(ep.port()) + "/query";

    auto[ec, response] = beauty::client().post(url, "THIS IS THE POST REQUEST BODY");

    CHECK_EQ(ec, boost::system::errc::success);
    CHECK_EQ(response.body(), "POST VERB - REQUEST BODY SIZE=29");
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(BigRequestResponseFixture, "Long request")
{
    std::string url = "http://127.0.0.1:" + std::to_string(ep.port()) + "/query";

    for(size_t s : sizes) {
        auto[ec, response] = beauty::client().post(url, std::string(s, '!'));

        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body(), "POST VERB - REQUEST BODY SIZE=" + std::to_string(s));
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(BigRequestResponseFixture, "Long response")
{
    std::string url = "http://127.0.0.1:" + std::to_string(ep.port()) + "/query";


    for(size_t s : sizes) {
        auto[ec, response] = beauty::client().post(url + "?size=" + std::to_string(s), "");

        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body().size(), s);
    }
}

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(BigRequestResponseFixture, "Long request and response")
{
    std::string url = "http://127.0.0.1:" + std::to_string(ep.port()) + "/query";

    for(size_t s : sizes) {
        auto[ec, response] = beauty::client().post(
                url + "?size=" + std::to_string(s),
                std::string(s, '!'));

        CHECK_EQ(ec, boost::system::errc::success);
        CHECK_EQ(response.body().size(), s);
    }
}
