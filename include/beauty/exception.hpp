#pragma once

#include <boost/beast/http/status.hpp>

#include <stdexcept>
#include <string>
#include <memory>

namespace beast = boost::beast;

namespace beauty {
class request;
class response;

// --------------------------------------------------------------------------
class exception : public std::exception {
public:
    exception(std::string message = "") : _message(std::move(message))
    {}

    exception(unsigned code, std::string message = "") :
            _error_code(beast::http::int_to_status(code)),
            _message(std::move(message))
    {}

    exception(beast::http::status code, std::string message = "") :
            _error_code(code),
            _message(std::move(message))
    {}

    unsigned code() const noexcept { return (unsigned)_error_code; }
    const char* what() const noexcept override { return _message.c_str(); }

    std::shared_ptr<response> create_response(const request& req) const;

protected:
    beast::http::status _error_code{beast::http::status::unknown};
    std::string _message;
};

// --------------------------------------------------------------------------
#define DEFINE_BEAUTY_EXCEPTION(NAME) \
class  NAME  : public exception { \
public: NAME(std::string message = "") : exception(beast::http::status::NAME, std::move(message)) {} }

// Client
DEFINE_BEAUTY_EXCEPTION(bad_request);
DEFINE_BEAUTY_EXCEPTION(unauthorized);
DEFINE_BEAUTY_EXCEPTION(forbidden);
DEFINE_BEAUTY_EXCEPTION(not_found);

// Server
DEFINE_BEAUTY_EXCEPTION(internal_server_error);
DEFINE_BEAUTY_EXCEPTION(not_implemented);
DEFINE_BEAUTY_EXCEPTION(bad_gateway);
DEFINE_BEAUTY_EXCEPTION(service_unavailable);

#undef DEFINE_BEAUTY_EXCEPTION
}
