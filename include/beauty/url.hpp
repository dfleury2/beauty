#pragma once

#include <beauty/export.hpp>

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
class BEAUTY_EXPORT url
{
public:
    url() = default;
    explicit url(std::string u);

    std::string_view scheme_view() const { return _scheme; }
    std::string scheme() const { return std::string(_scheme); }
    bool is_http() const { return _scheme == "http"; }
    bool is_https() const { return _scheme == "https"; }
    bool is_ws() const { return _scheme == "ws"; }
    bool is_wss() const { return _scheme == "wss"; }

    std::string_view login_view() const { return _login; }
    std::string login() const { return std::string(_login); }
    std::string_view password_view() const { return _password; }
    std::string password() const { return std::string(_password); }

    std::string_view host_view() const { return _host; }
    std::string host() const { return std::string(_host); }
    std::string_view port_view() const { return _port_view; }
    unsigned short   port() const { return _port; }

    std::string_view path_view() const { return (_path.size() ? _path : "/"); }
    std::string path() const { return std::string(_path.size() ? _path : "/"); }
    std::string_view query_view() const { return _query; }
    std::string query() const { return std::string(_query); }

    std::string strip_login_password() const;

private:
    // Full input url for the string_view
    std::string _url;

    // Http or https
    std::string_view    _scheme;
    // User Info
    std::string_view    _login;
    std::string_view    _password;

    std::string_view    _host;
    std::string_view    _port_view;
    unsigned short      _port{0};

    std::string_view    _path;
    std::string_view    _query;
};

}
