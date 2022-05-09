#include <beauty/beauty.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
std::string
read_file_content(const fs::path& filename, bool binary = false)
{
    std::ifstream file{filename, (binary ? std::ios_base::binary : std::ios_base::in)};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

//------------------------------------------------------------------------------
int
main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4) {
        std::cerr <<
            "Usage: " << argv[0] << " <address> <port> <doc_root>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " 0.0.0.0 8085 .\n";
        return EXIT_FAILURE;
    }
    auto address        = argv[1];
    auto port           = std::stoi(argv[2]);
    fs::path doc_root   = argv[3];

    boost::asio::io_context ioc;
    beauty::application app(ioc);

    beauty::server server(app);

    server.add_route("/:filename")
        .get([&doc_root](const beauty::request& req, beauty::response& res) {
            auto filename = req.a("filename").as_string("index.html");

            res.set(beauty::content_type::text_html);
            res.body() = read_file_content(doc_root / filename);
        });

    server.listen(port, address);

    ioc.run();
}
