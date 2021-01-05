#include <beauty/utils.hpp>

#include <beauty/header.hpp>
#include <beauty/version.hpp>
#include <beauty/request.hpp>
#include <beauty/response.hpp>

#include <boost/beast/http.hpp>

#include <iostream>
#include <regex>

namespace http = boost::beast::http;

namespace {

//---------------------------------------------------------------------------
template<typename T>
std::vector<std::string_view>
split(const T& str, char sep)
{
    std::vector<std::string_view> columns;
    size_t begin = 0;
    while(begin < str.size()) {
        size_t end = str.find(sep, begin);
        if (end == T::npos)
            end = str.size();

        columns.push_back(std::string_view(&str[begin], end - begin));
        begin = end + 1;
    }
    if (str.empty() || *str.rbegin() == sep)
        columns.push_back(std::string_view(&str[0] + begin, 0));

    return columns;
}

inline
unsigned char
hexdigit_to_num(unsigned char c)
{ return (c < 'A' ? c - '0' : toupper(c) - 'A' + 10); }

}

namespace beauty {

//---------------------------------------------------------------------------
std::shared_ptr<response>
bad_request(const request& req, const char* message)
{
    auto res = std::make_shared<response>(http::status::bad_request, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = std::string(message);
    return res;
}

//---------------------------------------------------------------------------
std::shared_ptr<response>
not_found(const request& req)
{
    auto res = std::make_shared<response>(http::status::not_found, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = "The resource [" + std::string(req.method_string())
            + "] '" + req.target().to_string() + "' was not found.";
    return res;
}

//---------------------------------------------------------------------------
std::shared_ptr<response>
server_error(const request& req, const char* message)
{
    auto res = std::make_shared<response>(http::status::internal_server_error, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = std::string("An error occurred: '") + message + "'";
    return res;
}

//---------------------------------------------------------------------------
void
fail(boost::system::error_code ec, const char* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

//---------------------------------------------------------------------------
std::vector<std::string_view>
split(const std::string& str, char sep)
{
    return ::split<std::string>(str, sep);
}

//---------------------------------------------------------------------------
std::vector<std::string_view>
split(const std::string_view& str_view, char sep)
{
    return ::split<std::string_view>(str_view, sep);
}

//---------------------------------------------------------------------------
std::string
escape(const std::string& s)
{
    static const char* dec2hex = "0123456789ABCDEF";
    static std::regex unsafe("[^A-Za-z0-9\\-\\._~]");

    std::string escaped;
    auto cur = std::sregex_token_iterator(s.begin(), s.end(), unsafe);
    auto end = std::sregex_token_iterator();
    auto vbegin = s.begin();

    for ( ; cur != end; ++cur) {
        // Append valid chars
        escaped.append(vbegin, cur->first);

        for (auto it = cur->first; it != cur->second; ++it) {
            auto c = (unsigned char) ::toupper(*it);

            escaped += '%';
            escaped += dec2hex[c >> 4];
            escaped += dec2hex[c & 0x0F];
        }

        vbegin = cur->second;
    }

    escaped.append(vbegin, s.end());

    return escaped;
}

//---------------------------------------------------------------------------
std::string
unescape(const std::string& s)
{
    static std::regex escaped("%([0-9A-Fa-f]{2})");

    // Support urlencoded '+' --> ' '
    std::string t = std::regex_replace(s,
                                       std::regex("\\+"),
                                       " ");

    std::string unescaped;
    auto cur = std::sregex_token_iterator(t.begin(), t.end(), escaped);
    auto end = std::sregex_token_iterator();
    auto vbegin = t.cbegin();

    for ( ; cur != end; ++cur) {
        auto it = cur->first;

        // Append unescaped chars
        unescaped.append(vbegin, it);

        // Skip percent
        ++it;

        auto c = hexdigit_to_num(*it++) << 4;
        c |= hexdigit_to_num(*it++);

        unescaped += c;

        vbegin = cur->second;
    }

    unescaped.append(vbegin, t.cend());

    return unescaped;
}

}
