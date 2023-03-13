#pragma once

#include <beauty/application.hpp>
#include <beauty/router.hpp>
#include <beauty/endpoint.hpp>
#include <beauty/export.hpp>

#include <boost/asio.hpp>

#include <memory>

namespace asio = boost::asio;

namespace beauty {

//---------------------------------------------------------------------------
// Accepts incoming connections and launches the sessions
//---------------------------------------------------------------------------
class BEAUTY_EXPORT acceptor : public std::enable_shared_from_this<acceptor>
{
public:
    acceptor(application& app,
        beauty::endpoint& endpoint,
        const beauty::router& router);

    ~acceptor();

    // Start accepting incoming connections
    void run();
    void stop();

    void do_accept();
    void on_accept(boost::system::error_code ec);

private:
    application&                _app;
    asio::ip::tcp::acceptor     _acceptor;
    asio::ip::tcp::socket       _socket;
    const beauty::router&       _router;
};

}
