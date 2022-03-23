#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/swagger.hpp>
#include <beauty/websocket_context.hpp>

#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace beauty
{
// --------------------------------------------------------------------------
// Http callback
// --------------------------------------------------------------------------
using route_cb = std::function<void(const beauty::request& req, beauty::response& res)>;

// --------------------------------------------------------------------------
// Websocket callbacks
// --------------------------------------------------------------------------
using ws_on_connect_cb = std::function<void(const ws_context&)>;
using ws_on_receive_cb = std::function<void(const ws_context&, const char*, std::size_t, bool)>;
using ws_on_disconnect_cb = std::function<void(const ws_context& ctx)>;

// --------------------------------------------------------------------------
struct ws_handler {
    ws_on_connect_cb on_connect = [](const ws_context&){};
    ws_on_receive_cb on_receive = [](const ws_context&, const char*, std::size_t, bool) {};
    ws_on_disconnect_cb on_disconnect = [](const ws_context& ctx){};
};

// --------------------------------------------------------------------------
class route
{
public:
    explicit route(const std::string& path, route_cb&& cb = [](const auto& req, auto& res){});
    route(const std::string& path, const beauty::route_info& route_info, route_cb&& cb = [](const auto& req, auto& res){});
    route(const std::string& path, ws_handler&& handler);

    bool match(beauty::request& req, bool is_websocket = false) const noexcept;

    void execute(const beauty::request& req, beauty::response& res) const {
        _cb(req, res);
    }

    // Websocket
    void connect(const ws_context& ctx) const {
        _ws_handler.on_connect(ctx);
    }
    void receive(const ws_context& ctx, const char* data, std::size_t size, bool is_text) const {
        _ws_handler.on_receive(ctx, data, size, is_text);
    }
    void disconnect(const ws_context& ctx) const {
        _ws_handler.on_disconnect(ctx);
    }

    [[nodiscard]] const std::string& path() const noexcept { return _path; }
    [[nodiscard]] const std::vector<std::string>& segments() const noexcept { return _segments; }
    [[nodiscard]] const beauty::route_info& route_info() const noexcept { return _route_info; }

private:
    void extract_route_info();
    void update_route_info(const beauty::route_info& route_info);

private:
    std::string _path;
    std::vector<std::string> _segments;

    route_cb    _cb;
    bool        _is_websocket{false};
    ws_handler  _ws_handler;
    beauty::route_info  _route_info;
};

}
