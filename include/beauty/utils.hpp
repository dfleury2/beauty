#pragma once

#include <boost/system/system_error.hpp>
#include <beauty/export.hpp>

#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace beauty {
class request;
class response;

namespace helper {
//---------------------------------------------------------------------------
// Returns a bad request response
//---------------------------------------------------------------------------
BEAUTY_EXPORT
std::shared_ptr<response>
bad_request(const beauty::request& req, const char* message);

//---------------------------------------------------------------------------
// Returns a not found response
//---------------------------------------------------------------------------
BEAUTY_EXPORT
std::shared_ptr<response>
not_found(const beauty::request& req);

//---------------------------------------------------------------------------
// Returns a server error response
//---------------------------------------------------------------------------
BEAUTY_EXPORT
std::shared_ptr<response>
server_error(const beauty::request& req, const char* what);
}

//---------------------------------------------------------------------------
// Report a failure
//---------------------------------------------------------------------------
BEAUTY_EXPORT
void
fail(boost::system::error_code ec, const char* what);

// --------------------------------------------------------------------------
BEAUTY_EXPORT
std::vector<std::string_view>
split(const std::string& str, char sep = '/');

BEAUTY_EXPORT
std::vector<std::string_view>
split(const std::string_view& str_view, char sep = '/');

// --------------------------------------------------------------------------
BEAUTY_EXPORT std::string escape(const std::string& s);
BEAUTY_EXPORT std::string unescape(const std::string& s);
BEAUTY_EXPORT std::string make_uuid();

// --------------------------------------------------------------------------
BEAUTY_EXPORT void thread_set_name(const std::string& name);

}
