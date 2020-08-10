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
    server(int concurrency = 1);
    server(certificates&& c, int concurrency = 1);
    ~server();

    server& listen(int port = 0, const char* address = "0.0.0.0");
    void run();

    void stop();

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
