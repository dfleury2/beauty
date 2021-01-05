#include <beauty/beauty.hpp>

int main()
{
    // Create a server
    beauty::server server;

    // Add a default '/' route
    server.get("/", [](const auto& req, auto& res) {
        res.body() = "It's work ;) ... it works! :)";
    });

    // Add a '/person/:id' route
    server.get("/person/:id", [](const auto& req, auto& res) {
        auto id = req.a("id");
        res.body() = "You asked for the person id: " + id;
    });

    // Open the listening port
    server.listen(8085);

    // Run the event loop - Warning, add a new thread (to be updated may be)
    server.run();
}
