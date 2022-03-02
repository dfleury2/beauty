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

    beauty::repeat(0.5, []{ std::cout << "I am alive" << std::endl;});

    beauty::server server;

    // Register a GET handler
    server.add_route("/index.html")
        .get([&](const beauty::request& req, beauty::response& res) {
            res.postpone();

            beauty::after(5.0, [&]() {
                std::cout << "postpone is done" << std::endl;
                res.set(beauty::content_type::text_html);
                res.body() = file_index_html;
                res.done();
            });
        });

    // Listen
    server.listen(8085);

    // Run the event loop
    server.run();
}
