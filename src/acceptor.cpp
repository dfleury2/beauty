#include <beauty/acceptor.hpp>

#include <beauty/session.hpp>
#include <beauty/utils.hpp>

namespace beauty {

//---------------------------------------------------------------------------
acceptor::acceptor(
    application& app,
    asio::ip::tcp::endpoint& endpoint,
    const beauty::router& router) :
            _app(app),
            _acceptor(app.ioc()),
            _socket(app.ioc()),
            _router(router)
{
    boost::system::error_code ec;

    // Open the acceptor
    _acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        _app.stop();
        return;
    }

    // Allow address reuse
    _acceptor.set_option(asio::socket_base::reuse_address(true));
    if (ec) {
        fail(ec, "set_option");
        _app.stop();
        return;
    }

    // Bind to the server address
    _acceptor.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        _app.stop();
        return;
    }

    // Update server endpoint in case of dynamic port allocation
    endpoint.port(_acceptor.local_endpoint().port());

    // Start listening for connections
    _acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        _app.stop();
        return;
    }
}

//---------------------------------------------------------------------------
acceptor::~acceptor()
{
    stop();
}

//---------------------------------------------------------------------------
void
acceptor::run()
{
    if (!_acceptor.is_open()) {
        return;
    }
    do_accept();
}

//---------------------------------------------------------------------------
void
acceptor::stop()
{
    if (_acceptor.is_open()) {
        _acceptor.close();
    }
}

//---------------------------------------------------------------------------
void
acceptor::do_accept()
{
    _acceptor.async_accept(
        _socket,
        [me = shared_from_this()](auto ec) {
            me->on_accept(ec);
        });
}

//---------------------------------------------------------------------------
void
acceptor::on_accept(boost::system::error_code ec)
{
    if (ec == boost::system::errc::operation_canceled) {
        return; // Nothing to do anymore
    }

    if (ec) {
        fail(ec, "accept");
        _app.stop();
    }
    else {
        // Create the session SLL or not and run it
        if (_app.is_ssl_activated()) {
            std::make_shared<session_https>(_app.ioc(), std::move(_socket), _router, _app.ssl_context())->run();
        } else {
            std::make_shared<session_http>(_app.ioc(), std::move(_socket), _router)->run();
        }
    }

    // Accept another connection
    do_accept();
}

}
