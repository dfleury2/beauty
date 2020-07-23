#pragma once

#include <beauty/application.hpp>
#include <beauty/route.hpp>
#include <beauty/router.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace beast = boost::beast;

namespace beauty
{
// --------------------------------------------------------------------------
class server
{
public:
    server(int concurrency = 1);
    server(certificates&& c, int concurrency = 1);

    server& listen(int port, const char* address = "0.0.0.0");
    void run();

    server& get(std::string path, route_cb&& cb);
    server& put(std::string path, route_cb&& cb);
    server& post(std::string path, route_cb&& cb);
    server& options(std::string path, route_cb&& cb);
    server& del(std::string path, route_cb&& cb);

private:
    beauty::application&    _app;
    beauty::router          _router;
};

}
