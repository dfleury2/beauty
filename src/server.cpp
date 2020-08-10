#include <beauty/server.hpp>

#include <beauty/acceptor.hpp>
#include <beauty/session.hpp>
#include <beauty/application.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
server::server(int concurrency) :
        _app(beauty::application::Instance(concurrency))
{}

// --------------------------------------------------------------------------
server::server(certificates&& c, int concurrency) :
        _app(beauty::application::Instance(std::move(c), concurrency))
{}

// --------------------------------------------------------------------------
server::~server()
{
    stop();
}

// --------------------------------------------------------------------------
server&
server::listen(int port, const char* address)
{
    auto ip_address = asio::ip::make_address(address);

    _endpoint = asio::ip::tcp::endpoint{ip_address, (unsigned short)port};

    // Create and launch a listening port
    _acceptor = std::make_shared<beauty::acceptor>(_app, _endpoint, _router);
    _acceptor->run();

    return *this;
}

// --------------------------------------------------------------------------
void
server::run()
{
    _app.ioc().run();
}


// --------------------------------------------------------------------------
void
server::stop()
{
    if (_acceptor) {
        _acceptor->stop();
    }
}

// --------------------------------------------------------------------------
server&
server::get(std::string path, route_cb&& cb)
{
    _router.add_route(beast::http::verb::get, beauty::route(std::move(path), std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::put(std::string path, route_cb&& cb)
{
    _router.add_route(beast::http::verb::put, beauty::route(std::move(path), std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::post(std::string path, route_cb&& cb)
{
    _router.add_route(beast::http::verb::post, beauty::route(std::move(path), std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::options(std::string path, route_cb&& cb)
{
    _router.add_route(beast::http::verb::options, beauty::route(std::move(path), std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::del(std::string path, route_cb&& cb)
{
    _router.add_route(beast::http::verb::delete_, beauty::route(std::move(path), std::move(cb)));
    return *this;
}

}
