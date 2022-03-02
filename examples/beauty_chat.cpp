#include <beauty/beauty.hpp>

#include <string>
#include <unordered_map>

using Sessions = std::unordered_map<std::string /* UUID */, std::weak_ptr<beauty::websocket_session>>;
using Rooms = std::unordered_map<std::string /* ROOM */,  Sessions>;

Rooms rooms;

int main(int argc, char* argv[])
{
    beauty::server server;

    server.add_route("/chat/:room")
        .ws(beauty::ws_handler{
            .on_connect = [](const beauty::ws_context& ctx) {
                rooms[ctx.attributes["room"].as_string()][ctx.uuid] = ctx.ws_session;
            },
            .on_receive = [](const beauty::ws_context& ctx, const char* data, std::size_t size, bool is_text) {
                for (auto& [uuid, session] : rooms[ctx.attributes["room"].as_string()]) {
                    if (auto s = session.lock(); s) {
                        s->send(std::string(data, size));
                    }
                }
            },
            .on_disconnect = [](const beauty::ws_context& ctx) {
                for (auto& [name, room] : rooms) {
                    room.erase(ctx.uuid);
                }
            }
        });

    server.listen(8085);

    beauty::wait();
}
