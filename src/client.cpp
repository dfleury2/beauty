#include <beauty/client.hpp>

#include <boost/system/error_code.hpp>
#if BEAUTY_ENABLE_OPENSSL
#include <boost/asio/ssl/stream.hpp>
#endif

#include "session_client.hpp"

namespace beauty
{
#if BEAUTY_ENABLE_OPENSSL
// --------------------------------------------------------------------------
client::client(certificates&& c)
{
    beauty::application::Instance(std::move(c));
}
#endif

// --------------------------------------------------------------------------
client::client_response
client::get_before(const beauty::duration& d, const std::string& url)
{
    // Set up an HTTP POST request message
    beauty::request request;
    request.method(beast::http::verb::get);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::get_before(const beauty::duration& d, const std::string& url, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::get);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::del_before(const beauty::duration& d, const std::string& url, std::string&& body)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);
    request.body() = std::move(body);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::del_before(const beauty::duration& d, const std::string& url, std::string&& body, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);
    request.body() = std::move(body);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::post_before(const beauty::duration& d, const std::string& url,
        std::string&& body)
{
    beauty::request request;
    request.method(beast::http::verb::post);
    request.body() = std::move(body);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::post_before(const beauty::duration& d, const std::string& url,
        std::string&& body, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::post);
    request.body() = std::move(body);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
client::client_response
client::put_before(const beauty::duration& d, const std::string& url,
        std::string&& body)
{
    // Set up an HTTP POST request message
    beauty::request request;
    request.method(beast::http::verb::put);
    request.body() = std::move(body);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::put_before(const beauty::duration& d, const std::string& url,
        std::string&& body, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::put);
    request.body() = std::move(body);

    send_request(std::move(request), d, url, std::move(cb));
}

// --------------------------------------------------------------------------
// Synchronous request
// --------------------------------------------------------------------------
client::client_response
client::send_request(beauty::request&& req, const beauty::duration& d,
        const std::string& url)
{
    asio::io_context ioc;

    boost::system::error_code ec;
    beauty::response response;

    try {
        std::shared_ptr<session_client_http> session_http;
#if BEAUTY_ENABLE_OPENSSL
        std::shared_ptr<session_client_https> session_https;
#endif
        auto _url = beauty::url(url);

        if (_url.is_https()) {
#if BEAUTY_ENABLE_OPENSSL
            session_https = std::make_shared<session_client_https>(ioc,
                    beauty::application::Instance().ssl_context());

            session_https->run(std::move(req), _url, d);
#else
            throw std::runtime_error("OpenSSL is not activated");
#endif
        }
        else {
            session_http = std::make_shared<session_client_http>(ioc);
            session_http->run(std::move(req), _url, d);
        }

        ioc.restart();
        ioc.run();

        response = std::move(
#if BEAUTY_ENABLE_OPENSSL
                _url.is_https() ? session_https->response():
#endif
                session_http->response());
    }
    catch(const boost::system::system_error& ex) {
        ec = ex.code();
    }
    catch(const std::exception&) {
        ec = boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category());
    }

    return std::make_pair(ec, response);
}

// --------------------------------------------------------------------------
// Asynchronous request
// --------------------------------------------------------------------------
void
client::send_request(beauty::request&& req, const beauty::duration& d,
        const std::string& url, client_cb&& cb)
{
    try {
        if (!beauty::application::Instance().is_started()) {
            beauty::application::Instance().start();
        }

        _url = beauty::url(url);

        if (_url.is_https()) {
#if BEAUTY_ENABLE_OPENSSL
            if (!_session_https) {
                // Create the session on first call...
                _session_https = std::make_shared<session_client_https>(
                        beauty::application::Instance().ioc(),
                        beauty::application::Instance().ssl_context());
            }

            _session_https->run(std::move(req), _url, d, std::move(cb));
#else
            throw std::runtime_error("OpenSSL is not activated");
#endif
        }
        else {
            if (!_session_http) {
                // Create the session on first call...
                _session_http = std::make_shared<session_client_http>(
                        beauty::application::Instance().ioc());
            }

            _session_http->run(std::move(req), _url, d, std::move(cb));
        }
    }
    catch(const boost::system::system_error& ex) {
        cb(ex.code(), {});
#if BEAUTY_ENABLE_OPENSSL
        _session_https.reset();
#endif
        _session_http.reset();
    }
    catch(const std::exception&) {
        cb(boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category()), {});
#if BEAUTY_ENABLE_OPENSSL
        _session_https.reset();
#endif
        _session_http.reset();
    }
}

// --------------------------------------------------------------------------
// WebSocket client management
// --------------------------------------------------------------------------
void
client::ws(const std::string& url, ws_handler&& handler)
{
    try {
        if (!beauty::application::Instance().is_started()) {
            beauty::application::Instance().start();
        }

        if (!_session_http) {
            // Create the session on first call...
            _websocket_client = std::make_shared<websocket_client>(
                    beauty::application::Instance().ioc(),
                    beauty::url(url),
                    std::move(handler));
        }

        ws_connect();
    }
    catch(const boost::system::system_error& ex) {
        handler.on_error(ex.code(), ex.what());
        _websocket_client.reset();
    }
    catch(const std::exception& ex) {
        handler.on_error(boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category()), ex.what());
        _websocket_client.reset();
    }
}

// --------------------------------------------------------------------------
void
client::ws_connect()
{
    _websocket_client->run();
}

// --------------------------------------------------------------------------
void
client::ws_send(std::string&& data)
{
    if (_websocket_client) {
        _websocket_client->send(std::move(data));
    }
}

}
