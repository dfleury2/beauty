#include <beauty/utils.hpp>

#include <beauty/header.hpp>
#include <beauty/version.hpp>

#include <iostream>

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
        columns.push_back(std::string_view(&str[begin], 0));

    return columns;
}

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
    res->body() = "The resource '" + req.target().to_string() + "' was not found.";
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
};

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

}
