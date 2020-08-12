//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

using namespace std::chrono;


//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    int _count = 0;
    int _step = 0;
    int _sent = 0;
    
    high_resolution_clock::time_point _start;

public:
    // Resolver and socket require an io_context
    explicit
    session(boost::asio::io_context& ioc)
        : resolver_(ioc)
        , socket_(ioc)
    {}
    
    ~session() {
        // Gracefully close the socket
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes so don't bother reporting it.
        if(ec && ec != boost::system::errc::not_connected) {
            fail(ec, "shutdown");
        }

        // If we get here then the connection is closed gracefully
    }
        

    // Start the asynchronous operation
    void
    run(
        char const* host,
        char const* port,
        char const* target,
        int version,
        int count)
    {
        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(http::verb::get);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        
        _count = count;
        _step = std::max(_count / 10, 1);

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            std::bind(
                &session::on_resolve,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_resolve(
        boost::system::error_code ec,
        tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(
            socket_,
            results.begin(),
            results.end(),
            std::bind(
                &session::on_connect,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_connect(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "connect");

        _start = high_resolution_clock::now();
        
        // Send the HTTP request to the remote host
        http::async_write(socket_, req_,
            std::bind(
                &session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        res_ = {};
        
        // Receive the HTTP response
        http::async_read(socket_, buffer_, res_,
            std::bind(
                &session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // Write the message to standard out
        //std::cout << res_ << std::endl;
        ++_sent;

        if (--_count > 0) {
            // Send the HTTP request to the remote host
            http::async_write(socket_, req_,
                std::bind(
                    &session::on_write,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2));
        }
        if (!(_count % _step)) {
            auto stop = high_resolution_clock::now();
            
            auto delay = duration_cast<duration<double>>(stop - _start).count();
            
            std::cout << _count << " left, "
                    << (int)(_sent / delay ) << " #/s"
                    << std::endl;
        }
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 4 && argc != 5 && argc != 6)
    {
        std::cerr <<
            "Usage: http-client-async <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>] [count]\n" <<
            "Example:\n" <<
            "    http-client-async www.example.com 80 /\n" <<
            "    http-client-async www.example.com 80 / 1.0\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    int version = (argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11);
    int count = (argc == 6 ? std::atoi(argv[5]) : 1);

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<session>(ioc)->run(host, port, target, version, count);

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();
}
