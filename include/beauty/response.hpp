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

    void set_header(beast::http::field name, const std::string& value) {
        set(name, value);
    }

    bool is_postponed() const { return _is_postponed; }
    void postpone() { _is_postponed = true; }

    void done() { _cb(); }

    void on_done(std::function<void()>&& cb) {
        _cb = std::move(cb);
    }

private:
    bool    _is_postponed = false;
    std::function<void()> _cb = [](){};
};
}
