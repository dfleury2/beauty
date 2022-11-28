#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/client.hpp>

// -----------------------------------------------------------------------------
TEST_CASE("Slow and Fast queries")
{
    std::string url = "http://0.0.0.0:22333";

    uint16_t port = 22333;
    std::string ip = "0.0.0.0";

    beauty::server server;

    std::atomic_bool done = false;

    server.add_route("/slow-response")
        .get([](const auto& req, auto& res) {
        std::cout <<"Sleep for 500 ms.." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });
    server.add_route("/fast-response")
        .get([](const auto& req, auto& res) {
        std::cout <<"Sleep for 100 ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    server.concurrency(2);
    server.listen(port, ip);

    std::cout << "calling slow API .. " << std::endl;
    beauty::client client1;
    client1.get(url + "/slow-response", [&](boost::system::error_code ec, beauty::response&& response){
        if (ec) {
            std::cout << "ERROR CODE = " << ec << ", " << ec.message() << std::endl;
        } else {
            std::cout << "DONE 1" << response.body() << std::endl;
        }
    });

    std::cout << "calling fast API .. " << std::endl;

    done = false;

    auto start_at = std::chrono::steady_clock::now();

    beauty::client client2;
    client2.get(url + "/fast-response", [&](boost::system::error_code ec, beauty::response&& response){
        done = true;
        if (ec) {
            std::cout << "ERROR CODE = " << ec << ", " << ec.message() << std::endl;
        } else {
            std::cout << "DONE 2" << response.body() << std::endl;
        }
    });

    // wait until fast api finish
    while (!done.load()) {std::this_thread::sleep_for(std::chrono::milliseconds(10));}

    auto fast_api_response_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_at).count();

    std::cout << "fast_api_response_time = " << fast_api_response_time << std::endl;

    CHECK(done);
    CHECK_GT(fast_api_response_time, 95);
    CHECK_LT(fast_api_response_time, 115);
}
