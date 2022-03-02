#include <beauty/beauty.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
std::string
read_file_content(const fs::path& filename, bool binary = false)
{
    std::ifstream file{filename, (binary ? std::ios_base::binary : std::ios_base::in)};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
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

    server.add_route("/:filename")
        .get([&doc_root](const beauty::request& req, beauty::response& res) {
            auto filename = req.a("filename").as_string("index.html");

            res.set(beauty::content_type::text_html);
            res.body() = read_file_content(doc_root / filename);
        });
    server.add_route("/:dirname/:filename")
        .get([&doc_root](const beauty::request& req, beauty::response& res) {
            auto dirname  = req.a("dirname").as_string();
            auto filename = req.a("filename").as_string();

            res.set(beauty::content_type::image_png);
            // or
            // res.set_header(beast::http::field::content_type, beauty::content_type::image_x_icon.value);
            res.body() = read_file_content(doc_root / dirname / filename, true);
        });
    server.add_route("/exception/:type")
        .get([](const beauty::request& req, beauty::response& res) {
            auto type = req.a("type").as_string("bad_request");

            if (type == "bad_request") {
                throw beauty::http_error::client::bad_request("This is a bad request");
            }
            else if (type == "unauthorized") {
                throw beauty::http_error::client::unauthorized("This is an unauthorized request");
            }
            else if (type == "forbidden") {
                throw beauty::http_error::client::forbidden("This is a forbidden request");
            }
            else if (type == "internal_server_error") {
                throw beauty::http_error::server::internal_server_error("This is an internal_server_error request");
            }
            else if (type == "not_implemented") {
                throw beauty::http_error::server::not_implemented("This is an not_implemented request");
            }
            else if (type == "bad_gateway") {
                throw beauty::http_error::server::bad_gateway("This is an bad_gateway request");
            }
            else if (type == "service_unavailable") {
                throw beauty::http_error::server::service_unavailable("This is an service_unavailable request");
            }

            throw beauty::http_error::client::bad_request("type [" + type + "] is not supported");
        });

    server.concurrency(threads);
    server.listen(port, address);

    beauty::wait();

}
