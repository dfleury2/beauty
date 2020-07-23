#pragma once

#include <beauty/router.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <string>
#include <memory>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty {

// --------------------------------------------------------------------------
// Handles an HTTP server connection
//---------------------------------------------------------------------------
class session : public std::enable_shared_from_this<session>
{
public:
    // Take ownership of the socket
    session(asio::ip::tcp::socket&& socket, const beauty::router& router);

    // Start the asynchronous operation
    void run();

    void do_read();

    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);

    void on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close);

    void do_close();

private:
    asio::ip::tcp::socket _socket;
    asio::strand<asio::io_context::executor_type> _strand;
    beast::flat_buffer  _buffer;
    beauty::request     _request;

    const beauty::router&   _router;
};

}
