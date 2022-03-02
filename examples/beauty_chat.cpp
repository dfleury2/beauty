#include <beauty/beauty.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

//------------------------------------------------------------------------------
using Sessions = std::unordered_map<std::string /* UUID */, std::weak_ptr<beauty::websocket_session>>;
using Rooms = std::unordered_map<std::string /* ROOM */,  Sessions>;

Rooms rooms;

//------------------------------------------------------------------------------
int
main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr <<
            "Usage: " << argv[0] << " <address> <port> <threads>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " 0.0.0.0 8085 5\n";
        return EXIT_FAILURE;
    }
    auto address        = argv[1];
    auto port           = std::stoi(argv[2]);
    auto threads        = std::max<int>(1, std::stoi(argv[3]));

    beauty::server server;

    server.add_route("/chat/:room")
        .ws(beauty::ws_handler{
            .on_connect = [](const beauty::ws_context& ctx) {
                std::cout << "on connect: " << ctx.uuid << std::endl;
                std::cout << "    target: " << ctx.target << std::endl;
                std::cout << "     route: " << ctx.route_path << std::endl;
                for (const auto& attr : ctx.attributes) {
                    std::cout << " attribute: " << attr.first << " = " << attr.second.as_string() << std::endl;
                }
                rooms[ctx.attributes["room"].as_string()][ctx.uuid] = ctx.ws_session;
            },
            .on_receive = [](const beauty::ws_context& ctx, const char* data, std::size_t size, bool is_text) {
                std::cout << "on receive: " << ctx.uuid << std::endl;

                for (auto& [uuid, session] : rooms[ctx.attributes["room"].as_string()]) {
                    if (auto s = session.lock(); s) {
                        s->send(std::string(data, size));
                    }
                }
            },
            .on_disconnect = [](const beauty::ws_context& ctx) {
                std::cout << "on disconnect: " << ctx.uuid << std::endl;
                for (auto& [name, room] : rooms) {
                    room.erase(ctx.uuid);
                }
            }
        });

    server.concurrency(threads);
    server.listen(port, address);

    beauty::wait();
}
