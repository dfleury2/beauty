#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/client.hpp>

#include <boost/json.hpp>

// --------------------------------------------------------------------------
struct SwaggerFixture
{
    SwaggerFixture()
    {
        beauty::server_info info;
        info.title = "SwaggerFixture";
        info.description = "SwaggerFixture description";
        info.version = "1.0";

        server.info(info);

        server.concurrency(2); // 2 threads are needed here because of synchronous get

        server.get("/index.html", [](const beauty::request& req, beauty::response& res) {
            auto filename = req.a("filename").as_string();

            res.set(beauty::content_type::text_plain);
            res.body() = "GET VERB";
            if (!filename.empty()) res.body() += ":" + filename;
        });

        server.get("/topic/:name", [](const beauty::request& req, beauty::response& res) {
            auto name = req.a("name").as_string();
            res.set(beauty::content_type::text_plain);
            res.body() = name;
        });

        server.post("/topic/:name", [](const beauty::request& req, beauty::response& res) {
            auto name = req.a("name").as_string();
            res.set(beauty::content_type::text_plain);
        });

        server.enable_swagger();
        server.listen();
        port = server.port();
    }

    ~SwaggerFixture() {
        server.stop();
    }

    beauty::server server;
    int port = 0;
};

// --------------------------------------------------------------------------
TEST_CASE_FIXTURE(SwaggerFixture, "Get Swagger server info")
{
    std::string url = "http://127.0.0.1:" + std::to_string(port) + "/swagger";

    auto[ec, response] = beauty::client().get(url);

    CHECK_EQ(ec, boost::system::errc::success);

    auto json = boost::json::parse(response.body());

    // /openapi
    CHECK_EQ(json.at("openapi").get_string(), "3.0.1");

    // /info
    CHECK_EQ(json.at("info").at("title").get_string(), "SwaggerFixture");
    CHECK_EQ(json.at("info").at("description").get_string(), "SwaggerFixture description");
    CHECK_EQ(json.at("info").at("version").get_string(), "1.0");
}
