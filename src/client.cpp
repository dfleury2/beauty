#include <beauty/client.hpp>

#include <boost/system/error_code.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "session_client.hpp"

namespace beauty
{

// --------------------------------------------------------------------------
client::client(certificates&& c)
{
    beauty::application::Instance(std::move(c));
}

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
client::del_before(const beauty::duration& d, const std::string& url)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);

    return send_request(std::move(request), d, url);
}

// --------------------------------------------------------------------------
void
client::del_before(const beauty::duration& d, const std::string& url, client_cb&& cb)
{
    beauty::request request;
    request.method(beast::http::verb::delete_);

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
    boost::system::error_code ec;
    beauty::response response;

    try {
        _url = beauty::url(url);

        if (beauty::application::Instance().is_ssl_activated()) {
            if (!_session_https) {
                // Create the session on first call...
                _session_https = std::make_shared<session_client_https>(_sync_ioc,
                        beauty::application::Instance().ssl_context());
            }

            _session_https->run(std::move(req), _url, d);
        }
        else {
            if (!_session_http) {
                _session_http = std::make_shared<session_client_http>(_sync_ioc);
            }

            _session_http->run(std::move(req), _url, d);
        }

        _sync_ioc.run();
        _sync_ioc.restart();

        response = std::move(beauty::application::Instance().is_ssl_activated() ?
                _session_https->response():
                _session_http->response());
    }
    catch(const boost::system::system_error& ex) {
        ec = ex.code();
    }
    catch(const std::exception& ex) {
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
        _url = beauty::url(url);

        if (beauty::application::Instance().is_ssl_activated()) {
            if (!_session_https) {
                // Create the session on first call...
                _session_https = std::make_shared<session_client_https>(
                        beauty::application::Instance().ioc(),
                        beauty::application::Instance().ssl_context());
            }

            _session_https->run(std::move(req), _url, d, std::move(cb));
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
    }
    catch(const std::exception& ex) {
        cb(boost::system::error_code(boost::system::errc::bad_address,
                boost::system::system_category()), {});
        return;
    }

}

}
