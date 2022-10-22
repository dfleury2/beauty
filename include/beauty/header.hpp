#pragma once
#include <string>

namespace beauty::header
{
    struct content_type {
        explicit content_type(const char* v) : value(v) {}
        std::string value;

        std::string operator()(const std::string&) const { return value; }
    };
}

namespace beauty::content_type {
    inline static const beauty::header::content_type text_plain         {"text/plain"};
    inline static const beauty::header::content_type text_html          {"text/html"};
    inline static const beauty::header::content_type application_json   {"application/json"};
    inline static const beauty::header::content_type image_x_icon       {"image/x-icon"};
    inline static const beauty::header::content_type image_png          {"image/png"};
}
