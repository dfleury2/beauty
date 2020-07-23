#include <beauty/beauty.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>


std::string
read_file_content(const std::string& filename)
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
    auto address  = argv[1];
    auto port     = static_cast<unsigned short>(std::atoi(argv[2]));
    std::string doc_root = argv[3];
    auto threads  = std::max<int>(1, std::atoi(argv[4]));

    std::string index_html = read_file_content(doc_root + "/index.html");
    std::string debian_ico = read_file_content(doc_root + "/icons/debian-logo.png");

    beauty::server s(threads);
    s.get("/index.html", [&index_html](const beauty::request& req, beauty::response& res) {
            res.set(beauty::content_type::text_html);
            res.body() = index_html;
        })
     .get("/icons/debian-logo.png",[&debian_ico](const beauty::request& req, beauty::response& res) {
            res.set(beauty::content_type::image_x_icon);
            res.body() = debian_ico;
        })
     .listen(port, address)
    ;


    std::cout << "Waiting a bit" << std::flush;
    //for(int i = 0; i < 10; ++i) {
    for(;;) {
        std::cout << "."; std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;
}
