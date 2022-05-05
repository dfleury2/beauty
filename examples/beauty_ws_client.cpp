#include <beauty/beauty.hpp>

#include <iostream>
#include <string>

//------------------------------------------------------------------------------
int
main(int argc, char* argv[])
{
    try {
        if (argc != 2) {
            throw std::runtime_error(
                      "Usage: " + std::string(argv[0]) + " url\n"
                      "Example:\n"
                      "    " + argv[0] + " ws://127.0.0.1:8085/chat/MyRoom");
        }

        beauty::client client;

        client.ws(argv[1], beauty::ws_handler{
            .on_connect = [](const beauty::ws_context& ctx) {
                std::cout << "--- Connected --- to: " << ctx.remote_endpoint << std::endl;
            },
            .on_receive = [](const beauty::ws_context& ctx, const char* data, std::size_t size, bool is_text) {
                std::cout << "--- Received:\n";
                std::cout.write(data, size);
                std::cout << "\n---" << std::endl;

            },
            .on_disconnect = [&client](const beauty::ws_context& ctx) {
                std::cout << "disconnection... retrying" << std::endl;
                beauty::after(1.0, [&client] {
                    std::cout << "Trying connection ..." << std::endl;
                    client.ws_connect();
                });
            },
            .on_error = [&client](boost::system::error_code ec, const char* what) {
                std::cout << "error: " << ec << ", " << ec.message() << ": " << what << std::endl;
            }
        });

        std::cout << "> ";
        std::cout.flush();
        for(std::string line; getline(std::cin, line);) {
            if (line == "q") break;
            std::cout << "> ";
            std::cout.flush();
            client.ws_send(std::move(line));
        }
    }
    catch(const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
