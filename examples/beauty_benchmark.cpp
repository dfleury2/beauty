#include <beauty/beauty.hpp>

#include <fstream>
#include <iterator>
#include <filesystem>

//------------------------------------------------------------------------------
std::string
read_file_content(const std::filesystem::path& filename)
{
    std::ifstream file{filename};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    // Read a file to server
    std::filesystem::path root = argv[1];
    auto file_index_html = read_file_content(root / "index.html");

    beauty::server server;

    // Register a GET handler
    server.get("/index.html",
        [&](const beauty::request& req, beauty::response& res) {
            res.set(beauty::content_type::text_html);
            res.body() = file_index_html;
        });

    // Listen - will start a thread in the app automatically
    server.listen(8085);

    // Wait for the server to stop - no additional loop
    server.wait();
}
