#include <beauty/utils.hpp>

#include <beauty/header.hpp>
#include <beauty/version.hpp>
#include <beauty/request.hpp>
#include <beauty/response.hpp>

#include <boost/beast/http.hpp>

#include <iostream>
#include <random>

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
int
hexdigit_to_num(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

}

namespace beauty {

namespace helper {
//---------------------------------------------------------------------------
std::shared_ptr<response>
bad_request(const request& req, const char* message) {
    auto res = std::make_shared<response>(http::status::bad_request, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = std::string(message);
    return res;
}

//---------------------------------------------------------------------------
std::shared_ptr<response>
not_found(const request& req) {
    auto res = std::make_shared<response>(http::status::not_found, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = "The resource [" + std::string(req.method_string())
                  + "] '" + std::string(req.target()) + "' was not found.";
    return res;
}

//---------------------------------------------------------------------------
std::shared_ptr<response>
server_error(const request& req, const char* message) {
    auto res = std::make_shared<response>(http::status::internal_server_error, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    res->body() = std::string("An error occurred: '") + message + "'";
    return res;
}
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
    static constexpr char dec2hex[] = "0123456789ABCDEF";
    std::string escaped;
    escaped.reserve(s.size());
    for (unsigned char c : s) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') {
            escaped += static_cast<char>(c);
        } else {
            escaped += '%';
            escaped += dec2hex[c >> 4];
            escaped += dec2hex[c & 0x0F];
        }
    }

    return escaped;
}

//---------------------------------------------------------------------------
std::string
unescape(const std::string& s)
{
    std::string unescaped;
    unescaped.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (c == '%' && s.size() - i >= 3) {
            const int high = hexdigit_to_num(s[i + 1]);
            const int low = hexdigit_to_num(s[i + 2]);
            if (high >= 0 && low >= 0) {
                unescaped += static_cast<char>((high << 4) | low);
                i += 2;
                continue;
            }
        }
        unescaped += (c == '+' ? ' ' : c);
    }

    return unescaped;
}

// --------------------------------------------------------------------------
std::string
make_uuid()
{
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* digit = "0123456789ABCDEF";

    std::string uuid(32, ' ');
    for (auto& c : uuid) {
        c = digit[dis(gen)];
    }
    return uuid;
}

//---------------------------------------------------------------------------
void
thread_set_name(const std::string& name) {
#if LINUX && _GNU_SOURCE
    constexpr int TASK_COMM_LEN = 16;

    char thread_name[TASK_COMM_LEN] = "";
    strncpy(thread_name, name.c_str(), sizeof(thread_name) - 1);

    pthread_setname_np(pthread_self(), thread_name);
#endif
}

}
