#include <beauty/beauty.hpp>

#include <iostream>

#include "read_file_content.hpp"

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    std::cout << "beauty server with swagger" << std::endl;

    // Check command line arguments.
    if (argc != 5) {
        std::cerr <<
            "Usage: " << argv[0] << " <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " 0.0.0.0 8085 . 5\n";
        return EXIT_FAILURE;
    }
    auto address        = argv[1];
    auto port           = std::stoi(argv[2]);
    fs::path doc_root   = argv[3];
    auto threads        = std::max<int>(1, std::stoi(argv[4]));

    // Http Server
    beauty::server server;

    server.enable_swagger("/swagger");

    server.get("/docs", [&](const auto& req, auto& res) {
        res.body() = R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Swagger UI</title>
                <link rel="stylesheet" type="text/css" href="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.1.3/swagger-ui.css">
            </head>
            <body>
                <div id="swagger-ui"></div>
                <script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.1.3/swagger-ui-bundle.js"></script>
                <script>
                const ui = SwaggerUIBundle({
                    url: '/swagger',
                    dom_id: '#swagger-ui',
                });
                </script>
            </body>
            </html>
        )";
        res.set(beauty::http::field::content_type, "text/html");
    });

    // sample routes
     server.get("/hello", [](const beauty::request& req, beauty::response& res) {
        res.body() = "Hello, World!";
        res.set(beauty::http::field::content_type, "text/plain");
    });

    server.get("/hello/:name", [](const beauty::request& req, beauty::response& res) {
        auto name = req.a("name").as_string();
        res.body() = "Hello, " + name + "!";
        res.set(beauty::http::field::content_type, "text/plain");
    });

    beauty::route_info param_info = {
        "Adding two numbers.",
        {
            {"a", "path", "The first number to add.", "string", "", true},
            {"b", "path", "The second number to add.", "string", "", true}
        }
    };
    server.get("/add/:a/:b", param_info, [](const beauty::request& req, beauty::response& res) {
        auto a = req.a("a").as_integer(0);
        auto b = req.a("b").as_integer(0);
        res.body() = "Sum: " + std::to_string(a + b);
        res.set(beauty::http::field::content_type, "text/plain");
    });

    server.concurrency(threads);
    server.listen(port, address);

    beauty::wait();
}
