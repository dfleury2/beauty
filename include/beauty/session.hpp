#pragma once

#include <beauty/router.hpp>
#include <beauty/version.hpp>
#include <beauty/utils.hpp>
#include <beauty/exception.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <string>
#include <memory>
#include <type_traits>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty {

// --------------------------------------------------------------------------
// Handles an HTTP/S server connection
//---------------------------------------------------------------------------
template<bool SSL>
class session : public std::enable_shared_from_this<session<SSL>>
{
public:
    using stream_type = std::conditional_t<SSL,
            asio::ssl::stream<asio::ip::tcp::socket&>,
            void*>;

public:
    template<bool U = SSL, typename std::enable_if_t<!U, int> = 0>
    session(asio::io_context& ioc, asio::ip::tcp::socket&& socket, const beauty::router& router) :
          _socket(std::move(socket)),
#if   (BOOST_VERSION < 107000)
          _strand(_socket.get_executor()),
#else
          _strand(asio::make_strand(ioc)),
#endif
          _router(router)
    {}

    template<bool U = SSL, typename std::enable_if_t<U, int> = 0>
    session(asio::io_context& ioc, asio::ip::tcp::socket&& socket, const beauty::router& router, asio::ssl::context& ctx) :
          _socket(std::move(socket)),
          _stream(_socket, ctx),
#if (BOOST_VERSION < 107000)
          _strand(_socket.get_executor()),
#else
          _strand(asio::make_strand(ioc)),
#endif
          _router(router)
    {}

    // Start the asynchronous operation
    void run()
    {
        if constexpr(SSL) {
            // Perform the SSL handshake
            _stream.async_handshake(
                asio::ssl::stream_base::server,
                asio::bind_executor(
                    _strand,
                    [me = this->shared_from_this()](auto ec) {
                        me->on_ssl_handshake(ec);
                }));
        } else {
            do_read();
        }
    }

    void on_ssl_handshake(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "failed handshake");
        }

        do_read();
    }

    void do_read()
    {
        //std::cout << "session: do read" << std::endl;
        // Make a new request_parser before reading
        _request_parser = std::make_unique<beast::http::request_parser<beast::http::string_body>>();
        _request_parser->body_limit(1024 * 1024 * 1024); // 1Go..

        // Read a full request (only if on _stream/_socket)
        if constexpr(SSL) {
            beast::http::async_read(_stream, _buffer, *_request_parser,
                asio::bind_executor(
                    _strand,
                    [me = this->shared_from_this()](auto ec, auto bytes_transferred) {
                        me->on_read(ec, bytes_transferred);
                    }));
        } else {
            beast::http::async_read(_socket, _buffer, *_request_parser,
                asio::bind_executor(
                    _strand,
                    [me = this->shared_from_this()](auto ec, auto bytes_transferred) {
                        me->on_read(ec, bytes_transferred);
                    }));
        }
    }

    void on_read(boost::system::error_code ec, std::size_t /* bytes_transferred */ )
    {
        //std::cout << "session: on read" << std::endl;

        // This means they closed the connection
        if (ec == beast::http::error::end_of_stream) {
            return do_close();
        }

        if (ec) {
            return fail(ec, "read");
        }

        // Send the response
        auto response = handle_request();

        if (!response->is_postponed()) {
            do_write(response);
        } else {
            response->on_done([me = this->shared_from_this(), response] {
                me->do_write(response);
            });
        }
    }

    void do_write(const std::shared_ptr<response>& response)
    {
        //std::cout << "session: do write" << std::endl;
        response->prepare_payload();

        // Write the response
        if constexpr(SSL) {
            beast::http::async_write(
                this->_stream,
                *response,
                asio::bind_executor(this->_strand,
                        [me = this->shared_from_this(), response](auto ec, auto bytes_transferred) {
                            me->on_write(ec, bytes_transferred, response->need_eof());
                        }
                )
            );
        } else {
            beast::http::async_write(
                this->_socket,
                *response,
                asio::bind_executor(this->_strand,
                        [me = this->shared_from_this(), response](auto ec, auto bytes_transferred) {
                            me->on_write(ec, bytes_transferred, response->need_eof());
                        }
                )
            );
        }
    }

    void on_write(boost::system::error_code ec, std::size_t /* bytes_transferred */, bool close)
    {
        //std::cout << "session: do write" << std::endl;
        if (ec) {
            return fail(ec, "write");
        }

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        //std::cout << "session: Read another request" << std::endl;
        // Allow to stay alive the session in case of postponed response
        do_read();
    }

    void do_close()
    {
        //std::cout << "session: do close, Shutdown the connection" << std::endl;
        if constexpr(SSL) {
            // Perform the SSL shutdown
            _stream.async_shutdown(
                asio::bind_executor(
                    _strand,
                    [me = this->shared_from_this()](auto ec){
                        me->on_ssl_shutdown(ec);
                    }));
        } else {
            // Send a TCP shutdown
            boost::system::error_code ec;
            _socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
            _socket.close();
        }
    }

    void on_ssl_shutdown(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "shutdown");

        _stream.lowest_layer().close();
    }

private:
    asio::ip::tcp::socket _socket;
    stream_type                                   _stream = {};
    asio::strand<asio::io_context::executor_type> _strand;
    beast::flat_buffer  _buffer;
    beauty::request     _request;
    std::unique_ptr<beast::http::request_parser<beast::http::string_body>> _request_parser;

    const beauty::router&   _router;

private:
    std::shared_ptr<response>
    handle_request()
    {
        // Make sure we can handle the method
        _request = _request_parser->release();
        _request.remote(_socket.remote_endpoint());

        auto found_method = _router.find(_request.method());
        if (found_method == _router.end()) {
            return helper::bad_request(_request, "Not supported HTTP-method");
        }

        // Try to match a route for this request target
        for(auto&& route : found_method->second) {
            if (route.match(_request)) {
                // Match will update parameters request from the URL
                try {
                    auto res = std::make_shared<response>(beast::http::status::ok, _request.version());
                    res->set(beast::http::field::server, BEAUTY_PROJECT_VERSION);
                    res->keep_alive(_request.keep_alive());

                    route.execute(_request, *res); // Call the route user handler

                    return res;
                }
                catch(const beauty::exception& ex) {
                    return ex.create_response(_request);
                }
                catch(const std::exception& ex) {
                    return helper::server_error(_request, ex.what());
                }
            }
        }

        return helper::not_found(_request);
    }
};

// --------------------------------------------------------------------------
using session_http = session<false>;
using session_https = session<true>;

}
