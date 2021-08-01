#include <beauty/beauty.hpp>

int main()
{
    // Create a server
    beauty::server server;

    server.concurrency(5);

    server.get("/slow-response", [](const auto& req, auto& res) {
        std::cout <<"Sleep for 7 sec.." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(7));
    })
    .get("/fast-response", [](const auto& req, auto& res) {
        std::cout <<"Sleep for 2 sec.." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    });

    // Open the listening port
    server.listen(8085);

    // Run the event loop - Warning, add a new thread (to be updated may be)
    server.wait();  // or server.wait(); for no supplementary thread
}
