#pragma once

#include <beauty/certificate.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <vector>
#include <thread>
#include <optional>

namespace asio = boost::asio;

namespace beauty {

// --------------------------------------------------------------------------
class application {
public:
    application(int concurrency = 1);
    application(certificates&& c, int concurrency = 1);

    ~application();

    void run();
    bool is_running() const;

    void stop();

    asio::io_context& ioc() { return _ioc; }
    asio::ssl::context& ssl_context() { return _ssl_context; }
    bool is_ssl_activated() const { return (bool)_certificates; }

    static application& Instance(int concurrency = 1);
    static application& Instance(certificates&& c, int concurrency = 1);

private:
    asio::io_context            _ioc;
    asio::executor_work_guard<asio::io_context::executor_type> _work;
    asio::ssl::context          _ssl_context;
    std::optional<certificates> _certificates;

    std::vector<std::thread>    _threads;

private:
    void load_server_certificates();
};

// --------------------------------------------------------------------------
// Singleton direct access
// --------------------------------------------------------------------------
void run(int concurrency = 1);
void stop();
bool is_running();

}
