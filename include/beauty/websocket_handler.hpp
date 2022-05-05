#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/websocket_context.hpp>

#include <functional>

namespace beauty {
// --------------------------------------------------------------------------
// Websocket callbacks
// --------------------------------------------------------------------------
using ws_on_connect_cb = std::function<void(const ws_context&)>;
using ws_on_receive_cb = std::function<void(const ws_context&, const char*, std::size_t, bool)>;
using ws_on_disconnect_cb = std::function<void(const ws_context& ctx)>;
using ws_on_error_cb = std::function<void(boost::system::error_code, const char* what)>;

// --------------------------------------------------------------------------
struct ws_handler {
    ws_on_connect_cb on_connect = [](const ws_context&) { };
    ws_on_receive_cb on_receive = [](const ws_context&, const char*, std::size_t, bool) {};
    ws_on_disconnect_cb on_disconnect = [](const ws_context& ctx) { };
    ws_on_error_cb on_error = [](boost::system::error_code, const char* what) {};
};

}
