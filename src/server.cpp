#include <beauty/server.hpp>

#include <boost/json.hpp>
#include <boost/json/src.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
server::server() :
        _app(beauty::application::Instance())
{}

// --------------------------------------------------------------------------
server::server(beauty::application& app) :
        _app(app)
{}

// --------------------------------------------------------------------------
server::~server()
{
    stop();
}

// --------------------------------------------------------------------------
void
server::listen(int port, const std::string& address)
{
    if (!_app.is_started()) {
        _app.start(_concurrency);
    }

    auto ip_address = asio::ip::make_address(address);

    _endpoint = beauty::endpoint{ip_address, (unsigned short)port};

    // Create and launch a listening port
    _acceptor = std::make_shared<beauty::acceptor>(_app, _endpoint, _router);
    _acceptor->run();
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
void
server::run()
{
    _app.run();
}

// --------------------------------------------------------------------------
void
server::wait()
{
    _app.wait();
}

// --------------------------------------------------------------------------
server&
server::get(const std::string& path, route_cb&& cb)
{
    return get(path, {}, std::move(cb));
}

// --------------------------------------------------------------------------
server&
server::get(const std::string& path, const route_info& route_info, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::get,
            beauty::route(path, route_info, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::ws(const std::string& path, ws_handler&& handler)
{
    _router.add_route(
            beast::http::verb::get,
            beauty::route(path, std::move(handler)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::put(const std::string& path, route_cb&& cb)
{
    return put(path, {}, std::move(cb));
}

// --------------------------------------------------------------------------
server&
server::put(const std::string& path, const route_info& route_info, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::put,
            beauty::route(path, route_info, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::post(const std::string& path, route_cb&& cb)
{
    return post(path, {}, std::move(cb));
}

// --------------------------------------------------------------------------
server&
server::post(const std::string& path, const beauty::route_info& route_info, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::post,
            beauty::route(path, route_info, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::options(const std::string& path, route_cb&& cb)
{
    return options(path, {}, std::move(cb));
}

// --------------------------------------------------------------------------
server&
server::options(const std::string& path, const beauty::route_info& route_info, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::options,
            beauty::route(path, route_info, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
server&
server::del(const std::string& path, route_cb&& cb)
{
    return del(path, {}, std::move(cb));
}

// --------------------------------------------------------------------------
server&
server::del(const std::string& path, const beauty::route_info& route_info, route_cb&& cb)
{
    _router.add_route(
            beast::http::verb::delete_,
            beauty::route(path, route_info, std::move(cb)));
    return *this;
}

// --------------------------------------------------------------------------
void
server::enable_swagger(const char* swagger_entrypoint)
{
    get(swagger_entrypoint, { .description = "Swagger API description entrypoint" },
        [this](const beauty::request& req, beauty::response& response) {
        boost::json::object json_swagger = {
            {"openapi", "3.0.1"},
            {"info", {
                    {"title",       _server_info.title},
                    {"description", _server_info.description},
                    {"version",     _server_info.version}
                }
            }
        };

        auto to_lower = [](std::string s) { for(auto& c : s) c = std::tolower(c); return s; };

        boost::json::object paths;
        for (const auto&[verb, routes] : _router) {
            for (auto&& route : routes) {
                boost::json::object description = {
                        {"description", route.route_info().description}
                };

                if (!route.route_info().route_parameters.empty()) {
                    description["parameters"] = boost::json::array();
                    for (const auto& param : route.route_info().route_parameters) {
                        description["parameters"] = {
                                        {"name",        param.name},
                                        {"in",          param.in},
                                        {"description", param.description},
                                        {"required",    param.required},
                                        {"schema", {
                                                {"type", param.type},
                                                {"format", param.format}
                                            }
                                        }
                                };
                    }
                }

                paths[swagger_path(route)].emplace_object()[to_lower(std::string(to_string(verb)))] = std::move(description);
            }
        }

        json_swagger["paths"] = std::move(paths);

        std::string body = boost::json::serialize(json_swagger);
        //std::cout << body << std::endl;

        response.set(beauty::http::field::access_control_allow_origin, "*");
        response.set(beauty::content_type::application_json);
        response.body() = std::move(body);
    });
}

}
