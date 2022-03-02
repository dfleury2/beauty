#pragma once

#include <beauty/attributes.hpp>
#include <beauty/utils.hpp>

#include <boost/beast/http.hpp>
#include "endpoint.hpp"

namespace beast = boost::beast;

namespace beauty
{
// --------------------------------------------------------------------------
class request : public boost::beast::http::request<boost::beast::http::string_body>
{
public:
    using beast::http::request<beast::http::string_body>::request;
    using beast::http::request<beast::http::string_body>::operator=;

    attributes& get_attributes() { return _attributes; }
    const attributes& get_attributes() const { return _attributes; }
    const attribute& a(const std::string& key) const { return _attributes[key]; }

    const beauty::endpoint& remote() const { return _remote_ep; }
    void remote(beauty::endpoint ep) { _remote_ep = std::move(ep); }

private:
    beauty::attributes  _attributes;
    beauty::endpoint    _remote_ep;
};

}
