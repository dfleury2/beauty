#include <beauty/beauty.hpp>

#include <iomanip>

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5) {
        std::cerr <<
            "Usage: " << argv[0] << " <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto address  = asio::ip::make_address(argv[1]);
    auto port     = static_cast<unsigned short>(std::atoi(argv[2]));
    std::string doc_root = argv[3];
    auto threads  = std::max<int>(1, std::atoi(argv[4]));

    auto& app = beauty::application::Instance(threads);
    beauty::router router;

    asio::ip::tcp::endpoint ep{address, port};

    // Create and launch a listening port
    std::make_shared<beauty::acceptor>(app, ep, router)->run();

    std::cout << "Waiting a bit" << std::flush;

    //for(int i = 0; i < 10; ++i) {
    for(;;) {
        std::cout << "." << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;
}
