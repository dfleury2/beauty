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
    using client_cb = std::function<void(boost::system::error_code, beauty::response)>;

public:
    client() = default;

    // ---------------
    // Synchronous GET
    // ---------------
    std::pair<boost::system::error_code, beauty::response>
    get_before(const beauty::duration& d, const std::string& url);

    std::pair<boost::system::error_code, beauty::response>
    get(const std::string& url)
    {
        return get_before(std::chrono::milliseconds(0), url);
    }

    std::pair<boost::system::error_code, beauty::response>
    get_before(double seconds, const std::string& url)
    {
        return get_before(std::chrono::milliseconds((int)(seconds * 1000)), url);
    }

    // ----------------
    // Synchronous POST
    // ----------------
    std::pair<boost::system::error_code, beauty::response>
    post_before(const beauty::duration& d, const std::string& url, std::string&& body);

    std::pair<boost::system::error_code, beauty::response>
    post(const std::string& url, std::string&& body)
    {
        return post_before(std::chrono::milliseconds(0), url, std::move(body));
    }

    std::pair<boost::system::error_code, beauty::response>
    post_before(double seconds, const std::string& url, std::string&& body)
    {
        return post_before(std::chrono::milliseconds((int)(seconds * 1000)), url, std::move(body));
    }

private:
    url             _url;

    std::shared_ptr<session_client> _session;
};

}
