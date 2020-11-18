#include <beauty/exception.hpp>

#include <beauty/version.hpp>
#include <beauty/request.hpp>
#include <beauty/response.hpp>

namespace beauty {
// --------------------------------------------------------------------------
std::shared_ptr<response>
exception::create_response(const request &req) const
{
    auto res = std::make_shared<response>(_error_code, req.version());
    res->set(http::field::server, BEAUTY_PROJECT_VERSION);
    res->set(content_type::text_plain);
    res->keep_alive(req.keep_alive());
    if (!_message.empty()) {
        res->body() = _message;
    }
    return res;
}

}
