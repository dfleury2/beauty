#pragma once

#include <beauty/request.hpp>
#include <beauty/response.hpp>
#include <beauty/swagger.hpp>

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
    explicit route(const std::string& path, const beauty::route_info& route_info, route_cb&& cb = {});

    bool match(beauty::request& req) const noexcept;

    void execute(const beauty::request& req, beauty::response& res) const {
        _cb(req, res);
    }

    const std::string& path() const noexcept { return _path; }
    const std::vector<std::string>& segments() const noexcept { return _segments; }
    const beauty::route_info& route_info() const noexcept { return _route_info; }

private:
    void extract_route_info();
    void update_route_info(const beauty::route_info& route_info);

private:
    std::string _path;
    std::vector<std::string> _segments;

    route_cb    _cb;
    beauty::route_info  _route_info;
};

}
