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
application::application() :
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23)
{}

// --------------------------------------------------------------------------
application::application(certificates&& c) :
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _certificates(std::move(c))
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
application::start(int concurrency)
{
    // Prevent to run twice
    if (is_started()) {
        return;
    }

    // Run the I/O service on the requested number of threads
    _threads.resize(std::max(1, concurrency));
    for(auto& t : _threads) {
        t = std::thread([this] {
            this->_ioc.run();
        });
    }
}

// --------------------------------------------------------------------------
void
application::run() {
    _ioc.run();
}
// --------------------------------------------------------------------------
bool
application::is_started() const
{
    return (_threads.size() && _threads[0].joinable());
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
application&
application::Instance()
{
    std::call_once(flag, [] {
        g_application = std::make_unique<application>();
    });
    return *g_application;
}

// --------------------------------------------------------------------------
application&
application::Instance(certificates&& c)
{
    std::call_once(flag, [cert = std::move(c)]() mutable {
        g_application = std::make_unique<application>(std::move(cert));
    });
    return *g_application;
}

// --------------------------------------------------------------------------
void
start(int concurrency)
{
    application::Instance().start(concurrency);
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

// --------------------------------------------------------------------------
bool is_started()
{
    return application::Instance().is_started();
}

}
