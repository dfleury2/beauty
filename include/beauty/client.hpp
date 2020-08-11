#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/version.hpp>
#include <beauty/timer.hpp>
#include <beauty/utils.hpp>
#include <beauty/url.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <functional>

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace beauty
{
class session_client;

// --------------------------------------------------------------------------
class client
{
public:
    using client_cb = std::function<void(boost::system::error_code, beauty::response&&)>;
    using client_response = std::pair<boost::system::error_code, beauty::response>;

public:
    client() = default;

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
    del_before(const beauty::duration& d, const std::string& url);
    void del_before(const beauty::duration& d, const std::string& url, client_cb&& cb);

    client_response
    del(const std::string& url)
    {
        return del_before(std::chrono::milliseconds(0), url);
    }

    client_response
    del_before(double seconds, const std::string& url)
    {
        return del_before(std::chrono::milliseconds((int)(seconds * 1000)), url);
    }

    void del(const std::string& url, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds(0), url, std::move(cb));
    }

    void del_before(double seconds, const std::string& url, client_cb&& cb)
    {
        del_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(cb));
    }

    // ----
    // POST
    // ----
    client_response
    post_before(const beauty::duration& d, const std::string& url, std::string&& body);
    void post_before(const beauty::duration& d, const std::string& url, std::string&& body, client_cb&& cb);

    client_response
    post(const std::string& url, std::string&& body)
    {
        return post_before(std::chrono::milliseconds(0), url, std::move(body));
    }

    void post(const std::string& url, std::string&& body, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds(0), url, std::move(body), std::move(cb));
    }

    client_response
    post_before(double seconds, const std::string& url, std::string&& body)
    {
        return post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body));
    }

    void post_before(double seconds, const std::string& url, std::string&& body, client_cb&& cb)
    {
        post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body), std::move(cb));
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

private:
    url             _url;

    std::shared_ptr<session_client> _session;

};

}
