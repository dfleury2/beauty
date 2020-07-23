#pragma once

#include <beauty/application.hpp>
#include <beauty/router.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <memory>

namespace asio = boost::asio;

namespace beauty {

//---------------------------------------------------------------------------
// Accepts incoming connections and launches the sessions
//---------------------------------------------------------------------------
class acceptor : public std::enable_shared_from_this<acceptor>
{
public:
    acceptor(application& app,
        asio::ip::tcp::endpoint endpoint,
        const beauty::router& router);

    // Start accepting incoming connections
    void run();

    void do_accept();
    void on_accept(boost::system::error_code ec);

private:
    application&                _app;
    asio::ip::tcp::acceptor     _acceptor;
    asio::ip::tcp::socket       _socket;
    const beauty::router&       _router;
};

}
