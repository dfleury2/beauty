#include <beauty/server.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
server::server() :
        _app(beauty::application::Instance())
{}

// --------------------------------------------------------------------------
server::server(certificates&& c) :
        _app(beauty::application::Instance(std::move(c)))
{}

// --------------------------------------------------------------------------
server::~server()
{
    stop();
}

// --------------------------------------------------------------------------
server&
server::listen(int port, const std::string& address)
{
    if (_app.is_stopped()) {
        return *this;
    }

    if (!_app.is_started()) {
        _app.start();
    }

    auto ip_address = asio::ip::make_address(address);

    _endpoint = asio::ip::tcp::endpoint{ip_address, (unsigned short)port};

    // Create and launch a listening port
    _acceptor = std::make_shared<beauty::acceptor>(_app, _endpoint, _router);
    _acceptor->run();

    return *this;
}

// --------------------------------------------------------------------------
void
server::start(int concurrency)
{
    _app.start(concurrency);
}

// --------------------------------------------------------------------------
void
server::run()
{
    _app.run();
}

// --------------------------------------------------------------------------
void
server::stop()
{
    if (_acceptor) {
        _acceptor->stop();
    }
    _app.stop();
}

// --------------------------------------------------------------------------
server&
server::get(const std::string& path, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::get,
            beauty::route(path, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::put(const std::string& path, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::put,
            beauty::route(path, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::post(const std::string& path, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::post,
            beauty::route(path, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::options(const std::string& path, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::options,
            beauty::route(path, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::del(const std::string& path, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::delete_,
            beauty::route(path, std::move(cb)));
    return *this;
}

}
