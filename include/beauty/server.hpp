#pragma once

#include <beauty/application.hpp>
#include <beauty/route.hpp>
#include <beauty/router.hpp>
#include <beauty/acceptor.hpp>

#include <boost/asio.hpp>

#include <string>

namespace beauty
{
// --------------------------------------------------------------------------
class server
{
public:
    server();
    explicit server(certificates&& c);
    ~server();

    server& concurrency(int concurrency) { _concurrency = concurrency; return *this; }
    server& get(const std::string& path, route_cb&& cb);
    server& put(const std::string& path, route_cb&& cb);
    server& post(const std::string& path, route_cb&& cb);
    server& options(const std::string& path, route_cb&& cb);
    server& del(const std::string& path, route_cb&& cb);

    void listen(int port = 0, const std::string& address = "0.0.0.0");
    void stop();
    void run();

    const asio::ip::tcp::endpoint& endpoint() const { return _endpoint; }

private:
    beauty::application&    _app;
    int                     _concurrency{1};
    beauty::router          _router;
    std::shared_ptr<beauty::acceptor> _acceptor;

    asio::ip::tcp::endpoint _endpoint;
};

}
