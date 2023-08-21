#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/swagger.hpp>
#include <beauty/route.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Route with automatic swagger")
{
    beauty::route route("/topic/:name");

    REQUIRE_EQ(route.route_info().route_parameters.size(), 1);

    const auto& params = route.route_info().route_parameters;
    CHECK_EQ(params[0].name,    "name");
    CHECK_EQ(params[0].in,      "path");
    CHECK(params[0].required);
}

// --------------------------------------------------------------------------
TEST_CASE("Swagger route with single parameter")
{
#if __cplusplus >= 202002L
    beauty::route route("/topic/:name", {
        .description = "My Way",
        .route_parameters = {{.name = "name", .description = "A Name"}}
    });

    CHECK_EQ(route.route_info().description, "My Way");
    REQUIRE_EQ(route.route_info().route_parameters.size(), 1);

    const auto& params = route.route_info().route_parameters;
    CHECK_EQ(params[0].name,    "name");
    CHECK_EQ(params[0].in,      "path");
    CHECK_EQ(params[0].description,      "A Name");
    CHECK(params[0].required);
#endif
}

// --------------------------------------------------------------------------
TEST_CASE("Swagger complex route")
{
#if __cplusplus >= 202002L
    beauty::route route("/topic/:name/chapter/:chapter_number/page/:page_number", beauty::route_info{
            .description = "Get a book to read",
            .route_parameters = {
                {.name = "name", .description = "Book Name", .type = "string"},
                {.name = "chapter_number", .description = "Chapter Number", .type = "integer"},
                {.name = "page_number", .description = "Page Number", .type = "integer"},
                {.name = "format", .description = "Format to output", .required = false }
            }
    });

    CHECK_EQ(route.route_info().description, "Get a book to read");
    REQUIRE_EQ(route.route_info().route_parameters.size(), 4);

    const auto& params = route.route_info().route_parameters;
    CHECK_EQ(params[0].name,    "name");
    CHECK_EQ(params[0].in,      "path");
    CHECK_EQ(params[0].description, "Book Name");
    CHECK_EQ(params[0].type,    "string");

    CHECK_EQ(params[1].name,    "chapter_number");
    CHECK_EQ(params[1].in,      "path");
    CHECK_EQ(params[1].description, "Chapter Number");
    CHECK_EQ(params[1].type,    "integer");

    CHECK_EQ(params[2].name,    "page_number");
    CHECK_EQ(params[2].in,      "path");
    CHECK_EQ(params[2].description, "Page Number");
    CHECK_EQ(params[2].type,    "integer");

    CHECK_EQ(params[3].name,    "format");
    CHECK_EQ(params[3].in,      "query");
    CHECK_EQ(params[3].description, "Format to output");
    CHECK_EQ(params[3].required, false);
#endif
}

// --------------------------------------------------------------------------
TEST_CASE("Swagger path")
{
    CHECK_EQ(beauty::swagger_path(beauty::route("/topic")), "/topic");
    CHECK_EQ(beauty::swagger_path(beauty::route("/topic/:name")), "/topic/{name}");
    CHECK_EQ(beauty::swagger_path(beauty::route("/topic/:name/chapter/:number")), "/topic/{name}/chapter/{number}");
}
