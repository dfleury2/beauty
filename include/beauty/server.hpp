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
    server(certificates&& c);
    ~server();

    server& listen(int port = 0, const char* address = "0.0.0.0");
    void start(int concurrency = 2);
    void stop();
    void run();

    server& get(std::string path, route_cb&& cb);
    server& put(std::string path, route_cb&& cb);
    server& post(std::string path, route_cb&& cb);
    server& options(std::string path, route_cb&& cb);
    server& del(std::string path, route_cb&& cb);

    const asio::ip::tcp::endpoint& endpoint() const { return _endpoint; }

private:
    beauty::application&    _app;
    beauty::router          _router;
    std::shared_ptr<beauty::acceptor> _acceptor;

    asio::ip::tcp::endpoint _endpoint;
};

}
