#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>

#include <boost/system/system_error.hpp>
#include <boost/beast/core/string.hpp>

#include <vector>
#include <string>
#include <string_view>

namespace beauty {
//---------------------------------------------------------------------------
// Returns a bad request response
//---------------------------------------------------------------------------
response
bad_request(beauty::request& req, const char* message);

//---------------------------------------------------------------------------
// Returns a not found response
//---------------------------------------------------------------------------
response
not_found(beauty::request& req);

//---------------------------------------------------------------------------
// Returns a server error response
//---------------------------------------------------------------------------
response
server_error(beauty::request& req, const char* what);

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

}
