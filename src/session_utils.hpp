#pragma once

#include "utils.hpp"
#include "version.hpp"

#include <beauty/request.hpp>
#include <beauty/response.hpp>

namespace beauty {

//---------------------------------------------------------------------------
template<typename Send>
void
handle_request(
    const beauty::router& router,
    beauty::request&& req,
    Send&& send)
{
    // Make sure we can handle the method
    auto found_method = router.find(req.method());
    if (found_method == router.end()) {
        return send(bad_request(req, "Not supported HTTP-method"));
    }

    // Try to match a route for this request target
    for(auto&& route : found_method->second) {
        if (route.match(req)) {
            try {
                response res{beast::http::status::ok, req.version()};
                res.set(beast::http::field::server, BEAUTY_PROJECT_VERSION);
                res.keep_alive(req.keep_alive());

                route.execute(req, res); // Call the route user handler

                res.prepare_payload();

                send(std::move(res));
                return;
            }
            catch(const std::exception& ex) {
                return send(server_error(req, ex.what()));
            }
        }
    }

    return send(not_found(req));
}

}
