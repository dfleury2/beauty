#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/version.hpp>
#include <beauty/timer.hpp>
#include <beauty/utils.hpp>
#include <beauty/url.hpp>
#include <beauty/websocket_handler.hpp>
#include <beauty/websocket_client.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <functional>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty
{
// Ugly forward declaration to be removed...
template<bool SSL>
class session_client;

using session_client_http = session_client<false>;
using session_client_https = session_client<true>;

// --------------------------------------------------------------------------
class client
{
public:
    using client_cb = std::function<void(boost::system::error_code, beauty::response&&)>;
    using client_response = std::pair<boost::system::error_code, beauty::response>;

public:
    client() = default;
    explicit client(certificates&& c);

    // ---
    // GET
    // ---
    client_response get_before(const beauty::duration& d, const std::string& url);
    void get_before(const beauty::duration& d, const std::string& url, client_cb&& cb);

    client_response get(const std::string& url)
    {
        return get_before(std::chrono::milliseconds(0), url);
    }

    client_response get_before(double seconds, const std::string& url)
    {
        return get_before(std::chrono::milliseconds((int)(seconds * 1000)), url);
    }

    void get(const std::string& url, client_cb&& cb)
    {
        get_before(std::chrono::milliseconds(0), url, std::move(cb));
    }

    void get_before(double seconds, const std::string& url, client_cb&& cb)
    {
        get_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(cb));
    }

    // ------
    // DELETE
    // ------
    client_response
    del_before(const beauty::duration& d, const std::string& url, std::string&& body);
    void del_before(const beauty::duration& d, const std::string& url, std::string&& body, client_cb&& cb);

    client_response
    del(const std::string& url, std::string&& body = "")
    {
        return del_before(std::chrono::milliseconds(0), url, std::move(body));
    }

    client_response
    del_before(double seconds, const std::string& url, std::string&& body = "")
    {
        return del_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body));
    }

    void del(const std::string& url, std::string&& body, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds(0), url, std::move(body), std::move(cb));
    }

    void del(const std::string& url, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds(0), url, "", std::move(cb));
    }

    void del_before(double seconds, const std::string& url, std::string&& body, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body), std::move(cb));
    }

    void del_before(double seconds, const std::string& url, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds((int)(seconds * 1000)), url, "", std::move(cb));
    }

    // ----
    // POST
    // ----
    client_response
    post_before(const beauty::duration& d, const std::string& url, std::string&& body);
    void post_before(const beauty::duration& d, const std::string& url, std::string&& body, client_cb&& cb);

    client_response
    post(const std::string& url, std::string&& body = "")
    {
        return post_before(std::chrono::milliseconds(0), url, std::move(body));
    }

    void post(const std::string& url, std::string&& body, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds(0), url, std::move(body), std::move(cb));
    }

    void post(const std::string& url, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds(0), url, "", std::move(cb));
    }

    client_response
    post_before(double seconds, const std::string& url, std::string&& body = "")
    {
        return post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body));
    }

    void post_before(double seconds, const std::string& url, std::string&& body, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body), std::move(cb));
    }

    void post_before(double seconds, const std::string& url, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, "", std::move(cb));
    }

    // ---
    // PUT
    // ---
    client_response
    put_before(const beauty::duration& d, const std::string& url, std::string&& body);
    void put_before(const beauty::duration& d, const std::string& url, std::string&& body, client_cb&& cb);

    client_response
    put(const std::string& url, std::string&& body)
    {
        return put_before(std::chrono::milliseconds(0), url, std::move(body));
    }

    void put(const std::string& url, std::string&& body, client_cb&& cb)
    {
        put_before(std::chrono::milliseconds(0), url, std::move(body), std::move(cb));
    }

    client_response
    put_before(double seconds, const std::string& url, std::string&& body)
    {
        return put_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body));
    }

    void put_before(double seconds, const std::string& url, std::string&& body, client_cb&& cb)
    {
        put_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body), std::move(cb));
    }

    // Low level send request
    client_response
    send_request(beauty::request&& req, const beauty::duration& d, const std::string& url);

    void
    send_request(beauty::request&& req, const beauty::duration& d, const std::string& url, client_cb&& cb);

    // --------------------------------------------------------------------------
    // WebSocket client management
    // --------------------------------------------------------------------------
    void ws(const std::string& url, ws_handler&& handler);
    void ws_connect();
    void ws_send(std::string&& data);

private:
    url             _url;

    // Waiting for some improvements...no more double shared_ptr
    std::shared_ptr<session_client_http> _session_http;
    std::shared_ptr<session_client_https> _session_https;

    asio::io_context                _sync_ioc;

    // Websocket management
    std::shared_ptr<websocket_client> _websocket_client;
};

}
