#pragma once

#include <beauty/version.hpp>
#include <beauty/utils.hpp>
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
class websocket_client : public std::enable_shared_from_this<websocket_client> {
public:
    websocket_client(asio::io_context& ioc, url&& url, ws_handler&& handler) :
            _resolver(asio::make_strand(ioc)),
            _websocket(asio::make_strand(ioc)),
            _url(std::move(url)),
            _ws_handler(std::move(handler))
    {}

    ~websocket_client()
    {
        if (_websocket.is_open()) {
            boost::system::error_code ec;
            _websocket.close(beast::websocket::close_code::normal, ec);
        }
    }

    void run()
    {
        // Get port or the default one
        auto port_view = _url.port_view();
        if (port_view.empty()) {
            port_view = "80";
        }

        _resolver.async_resolve(
                _url.host(),
                port_view,
                beast::bind_front_handler(
                        &websocket_client::on_resolve,
                        shared_from_this()));
    }

    void send(std::string&& msg) {
        beast::net::post(
                _websocket.get_executor(),
                [me = this->shared_from_this(), str = std::move(msg)]() mutable {
                    me->do_write(std::move(str));
                });
    }

private:
    void on_resolve(beast::error_code ec, asio::ip::tcp::resolver::results_type results) {
        if (ec) {
            return fail(ec, "resolve");
        }

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(_websocket).async_connect(
                results,
                [me = this->shared_from_this()](auto ec, auto&& ep) {
                    me->on_connect(ec, ep);
                });
    }

    void on_connect(beast::error_code ec, asio::ip::tcp::resolver::results_type::endpoint_type ep) {
        if (ec) {
            return fail(ec, "connect");
        }

        //_ws_context.ws_session = shared_from_this();
        _ws_context.remote_endpoint = _websocket.next_layer().socket().remote_endpoint();
        _ws_context.local_endpoint = _websocket.next_layer().socket().local_endpoint();
        _ws_context.uuid = make_uuid();
        _ws_context.target = _url.path();
        _ws_context.route_path = _url.path();
        //_ws_context.attributes = _url.query();
        //std::cout << "websocket_client: " << _ws_context.uuid << " - on connect" << std::endl;

        _websocket.set_option(
                beast::websocket::stream_base::timeout::suggested(
                        beast::role_type::client));

        _websocket.set_option(beast::websocket::stream_base::decorator(
            [](beast::websocket::response_type& res) {
                res.set(beast::http::field::server, std::string(BEAUTY_PROJECT_VERSION) +" beauty websocket");
            }));

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        auto host = _url.host() + ':' + std::to_string(ep.port());

        // Perform the websocket handshake
        _websocket.async_handshake(host, _url.path(),
                [me = this->shared_from_this()](auto ec) {
                    me->on_handshake(ec);
                });
    }

    void on_handshake(beast::error_code ec)
    {
        //std::cout << "websocket_client: " << _ws_context.uuid << " - on handshake" << std::endl;
        if (ec) {
            return fail(ec, "handshake");
        }

        _ws_handler.on_connect(_ws_context);

        do_read();
    }

    void do_read()
    {
        //std::cout << "websocket_client: " << _ws_context.uuid << " - do read" << std::endl;
        _websocket.async_read(
                _buffer,
                [me = this->shared_from_this()](auto ec, std::size_t size) {
                    me->on_read(ec, size);
                });
    }

    void on_read(beast::error_code ec, std::size_t)
    {
        //std::cout << "websocket_client: " << _ws_context.uuid << " - on read" << std::endl;
        if (ec == beast::websocket::error::closed) {
            return;
        }

        if (ec) {
            return fail(ec, "read");
        }

        _ws_handler.on_receive(_ws_context, static_cast<const char*>(_buffer.cdata().data()), _buffer.size(), _websocket.got_text());
        _buffer.consume(_buffer.size());

        do_read();
    }

    void do_write(std::string&& str)
    {
        _queue.push_back(std::move(str));

        // Are we already writing?
        if(_queue.size() > 1) {
            return;
        }

        // We are not currently writing, so send this immediately
        _websocket.async_write(
                beast::net::buffer(_queue.front()),
                [me = this->shared_from_this()](auto ec, std::size_t size) {
                    me->on_write(ec, size);
                });
    }

    void on_write(beast::error_code ec, std::size_t)
    {
        //std::cout << "websocket_client: " << _ws_context.uuid << " - on write" << std::endl;
        if (ec) {
            return fail(ec, "write");
        }

        _queue.pop_front();

        if (!_queue.empty()) {
            _websocket.async_write(
                    beast::net::buffer(_queue.front()),
                    [me = this->shared_from_this()](auto ec, std::size_t size) {
                        me->on_write(ec, size);
                    });
        }
    }

    void
    fail(boost::system::error_code ec, const char* what)
    {
        _ws_handler.on_error(ec, what);

        if (!_ws_context.uuid.empty()) {
            _ws_handler.on_disconnect(_ws_context);
        }

        _ws_context.uuid.clear();
        if (_websocket.is_open()) {
            _websocket.close(beast::websocket::close_code::normal, ec);
        }
    }

private:
    asio::ip::tcp::tcp::resolver _resolver;
    beast::websocket::stream<beast::tcp_stream> _websocket;
    beast::flat_buffer _buffer;
    url _url;
    ws_handler _ws_handler;
    ws_context _ws_context;
    std::deque<std::string> _queue;
};

}
