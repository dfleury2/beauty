#include <beauty/session.hpp>

#include <beauty/request.hpp>
#include <beauty/response.hpp>

#include "utils.hpp"
#include "session_utils.hpp"
#include "version.hpp"

namespace beauty {

//---------------------------------------------------------------------------
session::session(asio::ip::tcp::socket&& socket, const beauty::router& router)
    : _socket(std::move(socket)),
      _strand(_socket.get_executor()),
      _router(router)
{}

//---------------------------------------------------------------------------
void
session::run()
{
    do_read();
}

//---------------------------------------------------------------------------
void
session::do_read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    _request = {};

    // Read a full request
    beast::http::async_read(_socket, _buffer, _request,
        asio::bind_executor(
            _strand,
            [me = shared_from_this()](auto ec, auto bytes_transferred) {
                me->on_read(ec, bytes_transferred);
            }));
}

//---------------------------------------------------------------------------
void
session::on_read(boost::system::error_code ec, std::size_t bytes_transferred)
{
    // This means they closed the connection
    if (ec == beast::http::error::end_of_stream) {
        return do_close();
    }

    if (ec) {
        return fail(ec, "read");
    }

    // Send the response
    handle_request(_router, std::move(_request), [this](auto response) {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<decltype(response)>(std::move(response));

            // Write the response
            beast::http::async_write(
                this->_socket,
                *sp,
                asio::bind_executor(this->_strand,
                        [me = shared_from_this(), sp](auto ec, auto bytes_transferred) {
                            me->on_write(ec, bytes_transferred, sp->need_eof());
                        }
                )
            );
        }
    );
}

//---------------------------------------------------------------------------
void
session::on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close)
{
    if (ec) {
        return fail(ec, "write");
    }

    if (close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Read another request
    do_read();
}

//---------------------------------------------------------------------------
void
session::do_close()
{
    // Send a TCP shutdown
    boost::system::error_code ec;
    _socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
}

}
