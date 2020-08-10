#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <functional>

namespace beauty
{

using route_cb = std::function<void(const beauty::request& req, beauty::response& res)>;

// --------------------------------------------------------------------------
class route
{
public:
    explicit route(const std::string& path, route_cb&& cb = {});

    bool match(beauty::request& req) const noexcept;

    void execute(const beauty::request& req, beauty::response& res) const {
        _cb(req, res);
    }

private:
    std::vector<std::string> _segments;

    route_cb    _cb;
};

}
