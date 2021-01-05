#pragma once

#include <boost/system/system_error.hpp>
#include <boost/beast/core/string.hpp>

#include <vector>
#include <string>
#include <string_view>

namespace beauty {
class request;
class response;

//---------------------------------------------------------------------------
// Returns a bad request response
//---------------------------------------------------------------------------
std::shared_ptr<response>
bad_request(const beauty::request& req, const char* message);

//---------------------------------------------------------------------------
// Returns a not found response
//---------------------------------------------------------------------------
std::shared_ptr<response>
not_found(const beauty::request& req);

//---------------------------------------------------------------------------
// Returns a server error response
//---------------------------------------------------------------------------
std::shared_ptr<response>
server_error(const beauty::request& req, const char* what);

//---------------------------------------------------------------------------
// Report a failure
//---------------------------------------------------------------------------
void
fail(boost::system::error_code ec, const char* what);

// --------------------------------------------------------------------------
std::vector<std::string_view>
split(const std::string& str, char sep = '/');

std::vector<std::string_view>
split(const std::string_view& str_view, char sep = '/');

// --------------------------------------------------------------------------
std::string
escape(const std::string& s);

std::string
unescape(const std::string& s);
}
