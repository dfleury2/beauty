#pragma once

#include <beauty/version.hpp>
#include <beauty/utils.hpp>
#include <beauty/route.hpp>
#include <beauty/websocket_context.hpp>

#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <memory>
#include <deque>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty {

// --------------------------------------------------------------------------
// Handles a WebSocket server connection
// --------------------------------------------------------------------------
class websocket_session : public std::enable_shared_from_this<websocket_session> {
public:
    explicit websocket_session(asio::ip::tcp::socket&& socket, const beauty::route& route) :
            _websocket(std::move(socket)),
            _route(route)
    {}

    ~websocket_session()
    {
        if (!_ws_context.uuid.empty()) {
            _route.disconnect(_ws_context);
        }
    }

    void run(const beauty::request& req)
    {
        _ws_context.remote_endpoint = _websocket.next_layer().socket().remote_endpoint();
        _ws_context.local_endpoint = _websocket.next_layer().socket().local_endpoint();
        _ws_context.target = std::string{req.target()};
        _ws_context.route_path = _route.path();
        _ws_context.attributes = req.get_attributes();

        _websocket.set_option(
                beast::websocket::stream_base::timeout::suggested(
                        beast::role_type::server));

        _websocket.set_option(beast::websocket::stream_base::decorator(
            [](beast::websocket::response_type& res) {
                res.set(beast::http::field::server, std::string(BEAUTY_PROJECT_VERSION) +" beauty websocket");
            }));

        _websocket.async_accept(
                req,
                [me = this->shared_from_this()](auto ec) {
                    me->on_accept(ec);
                });
    }

    void send(std::string&& msg, bool binary_mode = false) {
        beast::net::post(
                _websocket.get_executor(),
                [me = this->shared_from_this(), str = std::move(msg), binary_mode]() mutable {
                    me->do_write(std::move(str), binary_mode);
                });
    }

private:
    beast::websocket::stream<beast::tcp_stream> _websocket;
    beast::flat_buffer _buffer;
    const beauty::route& _route;
    ws_context  _ws_context;

    struct message {
        std::string buffer;
        bool binary_mode{false};
    };

    std::deque<message> _queue;

private:
    void on_accept(beast::error_code ec)
    {
        if (ec) {
            return fail(ec, "accept");
        }

        _ws_context.ws_session = shared_from_this();
        _ws_context.uuid = make_uuid();
        //std::cout << "websocket_session: " << _ws_context.uuid << " - on accept" << std::endl;

        _route.connect(_ws_context);

        do_read();
    }

    void do_read()
    {
        //std::cout << "websocket_session: " << _ws_context.uuid << " - do read" << std::endl;

        _websocket.async_read(
                _buffer,
                [me = this->shared_from_this()](auto ec, std::size_t size) {
                    me->on_read(ec, size);
                });
    }

    void on_read(beast::error_code ec, std::size_t)
    {
        //std::cout << "websocket_session: " << _ws_context.uuid << " - on read" << std::endl;
        if (ec == beast::websocket::error::closed)
            return;

        if (ec) {
            return fail(ec, "read");
        }

        _route.receive(_ws_context, static_cast<const char*>(_buffer.cdata().data()), _buffer.size(), _websocket.got_text());
        _buffer.consume(_buffer.size());

        do_read();
    }

    void do_write(std::string&& str, bool binary_mode)
    {
        message msg;
        msg.buffer = std::move(str);
        msg.binary_mode = binary_mode;
        _queue.push_back(std::move(msg));

        // Are we already writing?
        if(_queue.size() > 1) {
            return;
        }

        // We are not currently writing, so send this immediately
        _websocket.binary(_queue.front().binary_mode);

        _websocket.async_write(
                beast::net::buffer(_queue.front().buffer),
                [me = this->shared_from_this()](auto ec, std::size_t size) {
                    me->on_write(ec, size);
                });
    }

    void on_write(beast::error_code ec, std::size_t)
    {
        //std::cout << "websocket_session: " << _ws_context.uuid << " - on write" << std::endl;
        if (ec) {
            return fail(ec, "write");
        }

        _queue.pop_front();

        if (!_queue.empty()) {
            _websocket.binary(_queue.front().binary_mode);

            _websocket.async_write(
                    beast::net::buffer(_queue.front().buffer),
                    [me = this->shared_from_this()](auto ec, std::size_t size) {
                        me->on_write(ec, size);
                    });
        }
    }
};

}
