#include <doctest/doctest.h>

#include <beauty/server.hpp>
#include <beauty/application.hpp>

#include <boost/asio.hpp>

#include <chrono>
#include <exception>
#include <string>
#include <thread>

namespace asio = boost::asio;

namespace {

const std::string request =
    "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";

// Send a request, then close the socket with SO_LINGER(on, 0): the kernel
// sends a RST instead of a FIN. When the server handles the request, the
// socket is no longer connected.
void
send_and_reset(const asio::ip::tcp::endpoint& ep)
{
    asio::io_context ioc;
    asio::ip::tcp::socket client(ioc);
    client.connect(ep);
    asio::write(client, asio::buffer(request));
    client.set_option(asio::socket_base::linger(true, 0));
    client.close();
}

// Return the status line, or an empty string if the server is gone or does
// not answer in time. When the server io_context is no longer running, the
// connection is still accepted by the kernel backlog, so a blocking read
// would hang forever: use async operations with a deadline instead.
std::string
send_and_receive_status(const asio::ip::tcp::endpoint& ep,
                        std::chrono::milliseconds timeout = std::chrono::seconds(2))
{
    asio::io_context ioc;
    asio::ip::tcp::socket check(ioc);
    asio::streambuf buf;
    std::string line;

    check.async_connect(ep, [&](boost::system::error_code ec) {
        if (ec) return;
        asio::async_write(check, asio::buffer(request),
            [&](boost::system::error_code ec, std::size_t) {
                if (ec) return;
                asio::async_read_until(check, buf, "\r\n",
                    [&](boost::system::error_code ec, std::size_t) {
                        if (ec) return;
                        std::istream is(&buf);
                        std::getline(is, line);
                        if (!line.empty() && line.back() == '\r') line.pop_back();
                    });
            });
    });

    ioc.run_for(timeout);
    return line;
}

}

// --------------------------------------------------------------------------
// Same scenario as scripts/reset_client.py against beauty_io_context_server:
// a client sending a request followed by a TCP RST must not make an
// exception escape from the user io_context::run().
// --------------------------------------------------------------------------
TEST_CASE("Client reset after request - external context")
{
    asio::io_context ioc;
    beauty::application app(ioc);
    beauty::server server(app);

    server.add_route("/:filename")
        .get([](const beauty::request&, beauty::response& res) {
            res.set(beauty::content_type::text_html);
            res.body() = "<html>Hello</html>";
        });

    server.listen(0, "127.0.0.1");
    const asio::ip::tcp::endpoint ep(
        asio::ip::make_address("127.0.0.1"), server.endpoint().port());

    // Like the example: a bare ioc.run(), but the exception is captured
    // here instead of terminating the test process.
    std::exception_ptr escaped;
    std::string escaped_what;
    std::thread runner([&] {
        try {
            ioc.run();
        }
        catch(const std::exception& ex) {
            escaped = std::current_exception();
            escaped_what = ex.what();
        }
    });

    // Sanity check: the server answers before the reset
    CHECK_EQ(send_and_receive_status(ep), "HTTP/1.1 200 OK");

    send_and_reset(ep);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    INFO("exception escaped from ioc.run(): ", escaped_what);
    CHECK_FALSE(escaped);

    // The server must still be up and answering
    CHECK_EQ(send_and_receive_status(ep), "HTTP/1.1 200 OK");

    ioc.stop();
    runner.join();
}
