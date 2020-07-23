#pragma once

#include <beauty/attributes.hpp>

#include <boost/beast/http.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
class request : public boost::beast::http::request<boost::beast::http::string_body>
{
public:
    attributes& get_attributes() { return _attributes; }
    const std::string& a(const std::string& key) const { return _attributes[key]; }

private:
    beauty::attributes  _attributes;
};

}
