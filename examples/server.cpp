#include <beauty/beauty.hpp>

#include <unordered_map>
#include <istream>
#include <iostream>

std::unordered_map<std::string /* id */, std::string /* name */> storage;

int main()
{
    // Create a server
    beauty::server server;

    // Add a default '/' route
    server.add_route("/")
        .get([](const auto& req, auto& res) {
            res.body() = "It's work ;) ... it works! :)";
        });

    // '/person'
    server.add_route("/person")
        // Get the list of available ids
        .get([](const auto& req, auto& res) {

            auto id = req.a("id").as_string();
            if (id.empty()) {
                for (const auto& [id, _] : storage) {
                    res.body() += id + "\n";
                }
            }
            else {
                std::string response = "Id: " + id + " not found";
                if (auto found = storage.find(id); found != end(storage)) {
                    res.body() = "id: " + id + ", name: " + found->second + "\n";
                }
                else {
                    res.body() = "Id: " + id + " NOT FOUND";
                }
            }
        })
        // Add a id/name
        .post([](const auto& req, auto& res) {
            std::istringstream iss(req.body());

            std::string id, name;
            iss >> id >> name;
            storage[id] = name;
        });

    // '/person/:id'
    server.add_route("/person/:id")
        // Get id information
        .get([](const auto& req, auto& res) {
            auto id = req.a("id").as_string();
            std::string response = "Id: " + id + " not found";
            if (auto found = storage.find(id); found != end(storage)) {
                res.body() = "id: " + id + ", name: " + found->second + "\n";
            }
        });


    // Example for : wget 'http://localhost:8085/channel/setChannel/{"indoor":true,"outdoor":{"left":false,"right":true}}'
    server.add_route("/channel/setChannel/:value")
        .get([&](const auto& req, auto& res) {
          auto channel = req.a("value").as_string();

          std::cout << "value: " << channel << std::endl;
          // Must display:
          // value: {"indoor":true,"outdoor":{"left":false,"right":true}}

          res.body() = "ok";
        });

    // Open the listening port
    server.listen(8085);

    // Run the event loop - Warning, add a new thread (to be updated may be)
    server.run();  // or server.wait(); for no supplementary thread
}
