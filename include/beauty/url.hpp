#pragma once

#include <string>
#include <string_view>

namespace beauty
{
// --------------------------------------------------------------------------
// Almost like this https://en.wikipedia.org/wiki/URL
// <scheme>://[[login][:password]@]<host>[:port][/path][?query]
// Examples:
//    http://localhost.com
//    http://localhost.com/path
//    http://localhost.com/path?query=yes
//    http://localhost.com:8085/path?query=yes
//    http://login@localhost.com/path
//    http://login:pwd@localhost.com/path
// --------------------------------------------------------------------------
class url
{
public:
    url() = default;
    explicit url(std::string u);

    bool is_http() const { return _scheme == "http:"; }
    bool is_https() const { return _scheme == "https:"; }

    std::string_view login() const { return _login; }
    std::string_view password() const { return _password; }

    std::string_view host() const { return _host; }
    std::string_view port() const { return _port; }

    std::string_view path() const { return (_path.size() ? _path : "/"); }
    std::string_view query() const { return _query; }

private:
    // Full input url for the string_view
    std::string _url;

    // Http or https
    std::string_view    _scheme;
    // User Info
    std::string_view    _login;
    std::string_view    _password;

    std::string_view    _host;
    std::string_view    _port;

    std::string_view    _path;
    std::string_view    _query;
};

}
