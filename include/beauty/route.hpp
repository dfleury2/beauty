#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/swagger.hpp>
#include <beauty/websocket_context.hpp>
#include <beauty/websocket_handler.hpp>
#include <beauty/export.hpp>

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
class BEAUTY_EXPORT route
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
