#include <beauty/application.hpp>

#include <beauty/certificate.hpp>

#include <boost/asio.hpp>

#include <iostream>
#include <memory>

namespace {
std::once_flag flag;
std::unique_ptr<beauty::application> g_application;
}


namespace beauty {

// --------------------------------------------------------------------------
application&
application::Instance(int concurrency)
{
    std::call_once(flag, [concurrency] {
        g_application = std::make_unique<application>(concurrency);
        g_application->run();
    });
    return *g_application;
}

// --------------------------------------------------------------------------
application&
application::Instance(certificates&& c, int concurrency)
{
    std::call_once(flag, [concurrency, &c] {
        g_application = std::make_unique<application>(std::move(c), concurrency);
        g_application->run();
    });
    return *g_application;
}

// --------------------------------------------------------------------------
application::application(int concurrency) :
    _ioc(concurrency),
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _threads(concurrency)
{}

// --------------------------------------------------------------------------
application::application(certificates&& c, int concurrency) :
    _ioc(concurrency),
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _certificates(std::move(c)),
    _threads(concurrency)
{
    load_server_certificates();
}

// --------------------------------------------------------------------------
application::~application()
{
    stop();
}

// --------------------------------------------------------------------------
void
application::run()
{
    std::cout << "Starting beauty application - " << _threads.size() << " thread(s)." << std::endl;

    // Run the I/O service on the requested number of threads
    for(auto& t : _threads) {
        t = std::thread([this] {
            this->_ioc.run();
        });
    }
}

// --------------------------------------------------------------------------
void
application::stop()
{
    std::cout << "Waiting to stop beauty application threads..." << std::endl;
    _ioc.stop();

    for(auto&& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    _ioc.restart();

    std::cout << "Beauty application stopped." << std::endl;
}

// --------------------------------------------------------------------------
void
application::load_server_certificates()
{
    if (_certificates->password.size()) {
        _ssl_context.set_password_callback(
            [pwd = _certificates->password](std::size_t, asio::ssl::context_base::password_purpose)
            {
                return pwd;
            });
    }

    auto options = asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2;
    if (_certificates->temporary_dh.size()) {
        options |= asio::ssl::context::single_dh_use;
    }

    _ssl_context.set_options(options);

    _ssl_context.use_certificate_chain(
            asio::buffer(
                    _certificates->certificat_chain.data(),
                    _certificates->certificat_chain.size()));

    _ssl_context.use_private_key(
        asio::buffer(_certificates->private_key.data(), _certificates->private_key.size()),
        asio::ssl::context::file_format::pem);

    if (_certificates->temporary_dh.size()) {
        _ssl_context.use_tmp_dh(
                asio::buffer(_certificates->temporary_dh.data(), _certificates->temporary_dh.size()));
    }
}

// --------------------------------------------------------------------------
void
run()
{
    application::Instance().run();
}

// --------------------------------------------------------------------------
void
stop()
{
    application::Instance().stop();
}

}
