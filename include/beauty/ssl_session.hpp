#pragma once

#include <beauty/router.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <string>
#include <memory>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty {

// --------------------------------------------------------------------------
// Handles an HTTP server connection
//---------------------------------------------------------------------------
class ssl_session : public std::enable_shared_from_this<ssl_session>
{
public:
    // Take ownership of the socket
    ssl_session(asio::ip::tcp::socket&& socket, asio::ssl::context& ctx, const beauty::router& router);

    // Start the asynchronous operation
    void run();

    void on_handshake(boost::system::error_code ec);

    void do_read();

    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);

    void on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close);

    void do_close();

    void on_shutdown(boost::system::error_code ec);

private:
    asio::ip::tcp::socket _socket;
    asio::ssl::stream<asio::ip::tcp::socket&>     _stream;
    asio::strand<asio::io_context::executor_type> _strand;
    beast::flat_buffer  _buffer;
    beauty::request     _request;

    const beauty::router&   _router;
};

}
