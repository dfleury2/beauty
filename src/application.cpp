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
    _threads(std::max(1, concurrency))
{}

// --------------------------------------------------------------------------
application::application(certificates&& c, int concurrency) :
    _ioc(concurrency),
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _certificates(std::move(c)),
    _threads(std::max(1, concurrency))
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
    // Prevent to run twice
    if (is_running()) {
        return;
    }

    // Run the I/O service on the requested number of threads
    for(auto& t : _threads) {
        t = std::thread([this] {
            this->_ioc.run();
        });
    }
}

// --------------------------------------------------------------------------
bool
application::is_running() const
{
    return _threads[0].joinable();
}

// --------------------------------------------------------------------------
void
application::stop()
{
    _ioc.stop();

    for(auto&& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Prepare for the next start
    _ioc.restart();
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
run(int concurrency)
{
    application::Instance(concurrency).run();
    // May be, may be not...
    // application::Instance(concurrency).ioc().run();
}

// --------------------------------------------------------------------------
void
stop()
{
    application::Instance().stop();
}

// --------------------------------------------------------------------------
bool is_running()
{
    return application::Instance().is_running();
}

}
