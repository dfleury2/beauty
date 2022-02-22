#pragma once

#include <beauty/header.hpp>

#include <boost/beast/http.hpp>

namespace beast = boost::beast;

namespace beauty
{
namespace http = beast::http;

// --------------------------------------------------------------------------
class response : public beast::http::response<beast::http::string_body>
{
public:
    using beast::http::response<beast::http::string_body>::response;
    using beast::http::response<beast::http::string_body>::set;
    using beast::http::response<beast::http::string_body>::operator=;

    void set(const beauty::header::content_type& ct) {
        set(beast::http::field::content_type, ct.value);
    }

    void set_header(beast::http::field name, const std::string& value) {
        set(name, value);
    }

    bool is_postponed() const { return _is_postponed; }
    void postpone() { _is_postponed = true; }

    void done() { _cb(); _cb = []{}; }

    void on_done(std::function<void()>&& cb) {
        _cb = std::move(cb);
    }

    // Result alias and a common helper for no error
    http::status status() const { return result(); }
    bool is_status_ok() const { return status() == http::status::ok; }

private:
    bool    _is_postponed = false;
    std::function<void()> _cb = [](){};
};
}
