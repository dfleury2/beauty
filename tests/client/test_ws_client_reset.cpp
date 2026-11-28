#include <doctest/doctest.h>

#include <beauty/client.hpp> // websocket_client.hpp is not self-contained

#include <boost/asio.hpp>

#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <chrono>
#include <exception>
#include <string>
#include <thread>

namespace asio = boost::asio;

namespace {

// True when a connection is waiting in the listener accept queue
bool
has_pending_connection(asio::ip::tcp::acceptor& listener, int timeout_ms)
{
    pollfd pfd{};
    pfd.fd = listener.native_handle();
    pfd.events = POLLIN;
    return ::poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN);
}

// Find, in this process, the client TCP socket connected to 127.0.0.1:port
int
find_client_socket(unsigned short port)
{
    for (int fd = 3; fd < 1024; ++fd) {
        sockaddr_in peer{};
        socklen_t len = sizeof(peer);
        if (::getpeername(fd, reinterpret_cast<sockaddr*>(&peer), &len) == 0
            && peer.sin_family == AF_INET && ntohs(peer.sin_port) == port) {
            return fd;
        }
    }
    return -1;
}

}

// --------------------------------------------------------------------------
// The server resets the connection between the TCP connect completion and
// the call to websocket_client::on_connect: remote_endpoint() must not
// throw out of the user io_context::run().
//
// The listener never accepts: the kernel completes the handshake on its own
// and keeps the connection in the accept queue. Closing the listener then
// sends a RST to this pending connection.
//
// asio checks SO_ERROR when the connect completes: if the RST is already
// there, it reports ECONNRESET and on_connect takes the normal error path.
// The race is a RST arriving just after this check: it is simulated here by
// consuming SO_ERROR on the client socket before asio reads it, so asio sees
// a successful connect and on_connect runs on a reset socket.
// --------------------------------------------------------------------------
TEST_CASE("Websocket client - server reset before on_connect")
{
    asio::io_context server_ioc;
    asio::ip::tcp::acceptor listener(server_ioc,
        asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 0));
    const auto port = listener.local_endpoint().port();

    std::string error_what;
    beauty::ws_handler handler;
    handler.on_error = [&](boost::system::error_code, const char* what) {
        error_what = what;
    };

    asio::io_context ioc;
    auto client = std::make_shared<beauty::websocket_client>(
        ioc,
        beauty::url("ws://127.0.0.1:" + std::to_string(port) + "/ws"),
        std::move(handler));
    client->run();

    // Drive the client handler by handler until the TCP connection is
    // established (resolve done, connect issued), without running on_connect
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    bool connected = false;
    while (!connected && std::chrono::steady_clock::now() < deadline) {
        if (ioc.poll_one() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        connected = has_pending_connection(listener, 5);
    }
    REQUIRE(connected);

    const int client_fd = find_client_socket(port);
    REQUIRE(client_fd >= 0);

    // RST to the pending connection, and let it reach the client socket
    listener.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Consume the pending ECONNRESET, as if the RST came after asio's check
    int so_error = 0;
    socklen_t so_len = sizeof(so_error);
    REQUIRE(::getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &so_error, &so_len) == 0);
    CHECK_EQ(so_error, ECONNRESET);

    // Now on_connect runs on a reset socket
    std::string escaped_what;
    try {
        ioc.run_for(std::chrono::milliseconds(500));
    }
    catch(const std::exception& ex) {
        escaped_what = ex.what();
    }

    INFO("on_error: ", error_what);
    CHECK_MESSAGE(escaped_what.empty(),
        "exception escaped from ioc.run(): ", escaped_what);
}
