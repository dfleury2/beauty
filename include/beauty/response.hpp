#pragma once

#include <beauty/header.hpp>

#include <boost/beast/http.hpp>

namespace beast = boost::beast;

namespace beauty
{
// --------------------------------------------------------------------------
class response : public beast::http::response<beast::http::string_body>
{
public:
    using beast::http::response<beast::http::string_body>::response;
    using beast::http::response<beast::http::string_body>::set;

    void set(const beauty::header::content_type& ct) {
        set(beast::http::field::content_type, ct.value);
    }
};
}
