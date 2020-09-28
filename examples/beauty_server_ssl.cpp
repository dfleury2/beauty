#include <beauty/beauty.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
std::string
read_file_content(const fs::path& filename)
{
    std::ifstream file{filename};
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
    auto port           = std::atoi(argv[2]);
    fs::path doc_root   = argv[3];
    auto threads        = std::max<int>(1, std::atoi(argv[4]));

    // Read certificates
    beauty::certificates c;
    c.certificat_chain  = read_file_content("./certificat.pem");
    c.private_key       = read_file_content("./private_key.pem");
//    c.temporary_dh      = read_file_content(etc_dir + "/temp_dh.pem");
//    c.password          = "test";

    // Http Server
    beauty::server s(std::move(c));

    s.get("/:filename", [&doc_root](const beauty::request& req, beauty::response& res) {
            auto& filename = req.a("filename");

            res.set(beauty::content_type::text_html);
            res.body() = read_file_content(doc_root / filename);
        })
     .get("/:dirname/:filename",[&doc_root](const beauty::request& req, beauty::response& res) {
            auto& dirname  = req.a("dirname");
            auto& filename = req.a("filename");

            res.set(beauty::content_type::image_x_icon);
            res.body() = read_file_content(doc_root / dirname / filename);
        })
     .concurrency(threads)
     .listen(port, address);

    beauty::run();
}
