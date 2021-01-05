#include <beauty/beauty.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

using namespace std::chrono_literals;

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
    if (argc != 3) {
        std::cerr <<
            "Usage: " << argv[0] << " <url> <count>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " https://127.0.0.1:8085/index.html 5000\n";
        return EXIT_FAILURE;
    }

    auto url        = argv[1];
    int count       = std::atoi(argv[2]);
    int step        = count / 10;
    if (!step) step = 1;

    size_t message_size = 0;
    int failure  = 0;
    size_t total_bytes = 0;

    // Read certificates
    beauty::certificates c;
    c.certificat_chain  = read_file_content("./certificat.pem");
    c.private_key       = read_file_content("./private_key.pem");
//    c.temporary_dh      = read_file_content(etc_dir + "/temp_dh.pem");
//    c.password          = "test";

    auto start = std::chrono::high_resolution_clock::now();

    beauty::client client(std::move(c));

    int i = 0;
    for(; i < count; ++i) {
        auto [ec, response] = client.get(url);

        if (ec) {
            ++failure;
        } else {
            if (!i) {
                message_size = response.body().size();
            }
            total_bytes += response.body().size();

            if (count == 1) {
                std::cout << "[" << response.body().substr(0, 256) << "]" << std::endl;
            }
        }

        if (i && !(i % step)) {
            std::cout << i << " response(s) received" << std::endl;
        }
    }

    std::cout << "Document Length: " << message_size << std::endl;
    std::cout << std::endl;

    std::cout << i << " response(s) received" << std::endl;
    std::cout << failure << " failure(s)" << std::endl;
    std::cout << std::fixed << (total_bytes / 1024.0) <<  " Kbyte(s) received" << std::endl;

    auto stop = std::chrono::high_resolution_clock::now();

    auto delay = std::chrono::duration_cast<
            std::chrono::duration<double>>(stop - start).count();
    std::cout << "Total duration:      " << delay << " seconds" << std::endl;
    std::cout  << std::fixed << "Requests per seconds: " << (count / delay) << " [#/sec]" << std::endl;
}
