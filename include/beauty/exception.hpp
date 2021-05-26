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
    explicit exception(std::string message = "") : _message(std::move(message)) {}

    explicit exception(unsigned code, std::string message = "") :
            _error_code(beast::http::int_to_status(code)),
            _message(std::move(message)) {}

    explicit exception(beast::http::status code, std::string message = "") :
            _error_code(code),
            _message(std::move(message)) {}

    [[nodiscard]] unsigned code() const noexcept { return (unsigned) _error_code; }

    [[nodiscard]] const char* what() const noexcept override { return _message.c_str(); }

    [[nodiscard]] std::shared_ptr<response> create_response(const request& req) const;

protected:
    beast::http::status _error_code{beast::http::status::unknown};
    std::string _message;
};

// --------------------------------------------------------------------------
namespace http_error {

#define DEFINE_BEAUTY_EXCEPTION(NAME) \
class  NAME  : public beauty::exception { \
public: explicit NAME(std::string message = "") : exception(beast::http::status::NAME, std::move(message)) {} }

// Client - 400
namespace client {
DEFINE_BEAUTY_EXCEPTION(bad_request);
DEFINE_BEAUTY_EXCEPTION(unauthorized);
DEFINE_BEAUTY_EXCEPTION(payment_required);
DEFINE_BEAUTY_EXCEPTION(forbidden);
DEFINE_BEAUTY_EXCEPTION(not_found);
DEFINE_BEAUTY_EXCEPTION(method_not_allowed);
DEFINE_BEAUTY_EXCEPTION(not_acceptable);
DEFINE_BEAUTY_EXCEPTION(proxy_authentication_required);
DEFINE_BEAUTY_EXCEPTION(request_timeout);
DEFINE_BEAUTY_EXCEPTION(conflict);
DEFINE_BEAUTY_EXCEPTION(gone);
DEFINE_BEAUTY_EXCEPTION(length_required);
DEFINE_BEAUTY_EXCEPTION(precondition_failed);
DEFINE_BEAUTY_EXCEPTION(payload_too_large);
DEFINE_BEAUTY_EXCEPTION(uri_too_long);
DEFINE_BEAUTY_EXCEPTION(unsupported_media_type);
DEFINE_BEAUTY_EXCEPTION(range_not_satisfiable);
DEFINE_BEAUTY_EXCEPTION(expectation_failed);
DEFINE_BEAUTY_EXCEPTION(misdirected_request);
DEFINE_BEAUTY_EXCEPTION(unprocessable_entity);
DEFINE_BEAUTY_EXCEPTION(locked);
DEFINE_BEAUTY_EXCEPTION(failed_dependency);
DEFINE_BEAUTY_EXCEPTION(upgrade_required);
DEFINE_BEAUTY_EXCEPTION(precondition_required);
DEFINE_BEAUTY_EXCEPTION(too_many_requests);
DEFINE_BEAUTY_EXCEPTION(request_header_fields_too_large);
DEFINE_BEAUTY_EXCEPTION(connection_closed_without_response);
DEFINE_BEAUTY_EXCEPTION(unavailable_for_legal_reasons);
DEFINE_BEAUTY_EXCEPTION(client_closed_request);
}

// Server - 500
namespace server {
DEFINE_BEAUTY_EXCEPTION(internal_server_error);
DEFINE_BEAUTY_EXCEPTION(not_implemented);
DEFINE_BEAUTY_EXCEPTION(bad_gateway);
DEFINE_BEAUTY_EXCEPTION(service_unavailable);
DEFINE_BEAUTY_EXCEPTION(gateway_timeout);
DEFINE_BEAUTY_EXCEPTION(http_version_not_supported);
DEFINE_BEAUTY_EXCEPTION(variant_also_negotiates);
DEFINE_BEAUTY_EXCEPTION(insufficient_storage);
DEFINE_BEAUTY_EXCEPTION(loop_detected);
DEFINE_BEAUTY_EXCEPTION(not_extended);
DEFINE_BEAUTY_EXCEPTION(network_authentication_required);
DEFINE_BEAUTY_EXCEPTION(network_connect_timeout_error);
}

#undef DEFINE_BEAUTY_EXCEPTION
}
}
