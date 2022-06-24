#include <beauty/application.hpp>

#include <beauty/timer.hpp>

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
        _work(asio::make_work_guard(_ioc))
{
#if BEAUTY_ENABLE_OPENSSL
    _ssl_context.set_verify_mode(asio::ssl::verify_none);
#endif
}

// --------------------------------------------------------------------------
application::application(asio::io_context& ioc) :
        _ioc_external(&ioc),
        _work(asio::make_work_guard(*_ioc_external))
{
#if BEAUTY_ENABLE_OPENSSL
    _ssl_context.set_verify_mode(asio::ssl::verify_none);
#endif
}

#if BEAUTY_ENABLE_OPENSSL
// --------------------------------------------------------------------------
application::application(certificates&& c) :
        _work(asio::make_work_guard(_ioc)),
        _certificates(std::move(c))
{
    load_server_certificates();
}

// --------------------------------------------------------------------------
application::application(asio::io_context& ioc, certificates&& c) :
        _ioc_external(&ioc),
        _work(asio::make_work_guard(*_ioc_external)),
        _certificates(std::move(c))
{
    load_server_certificates();
}
#endif

// --------------------------------------------------------------------------
application::~application()
{
    stop(false);
}

// --------------------------------------------------------------------------
void
application::start(int concurrency)
{
    // Prevent to run twice
    if (is_started() || !is_ioc_owner()) {
        return;
    }

    if (is_stopped()) {
        // The application was started before, we need
        // to restart the ioc cleanly
        ioc().restart();
    }
    _state = State::started;

    // Run the I/O service on the requested number of threads
    _threads.resize(std::max(1, concurrency));
    _active_threads = 0;
    for(auto& t : _threads) {
        ++_active_threads;
        t = std::thread([this] {
            for(;;) {
                try {
                    ioc().run();
                    break;
                }
                catch(const std::exception& ex) {
                    std::cout << "worker error: " << ex.what() << std::endl;
                }
            }
            --_active_threads;
        });
        t.detach();
            // Threads are detached, it's easier to stop inside an handler
    }
}

// --------------------------------------------------------------------------
void
application::stop(bool reset)
{
    if (is_stopped() || !is_ioc_owner()) {
        return;
    }
    _state = State::stopped;

    if (reset) {
        for(auto&& t : timers) {
            t->stop();
        }
        timers.clear();
    }

    ioc().stop();

    while(_active_threads != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// --------------------------------------------------------------------------
void
application::run()
{
    if (!is_ioc_owner()) return;

    if (is_stopped()) {
        ioc().restart();
    }
    _state = State::started;

    // Run
    ioc().run();
}

// --------------------------------------------------------------------------
void
application::wait() const
{
    if (!is_ioc_owner()) return;

    while(!is_stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

// --------------------------------------------------------------------------
void
application::post(std::function<void()> fct)
{
    boost::asio::post(ioc().get_executor(), std::move(fct));
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

#if BEAUTY_ENABLE_OPENSSL
// --------------------------------------------------------------------------
void
application::load_server_certificates()
{
    if (!_certificates->password.empty()) {
        _ssl_context.set_password_callback(
            [pwd = _certificates->password](std::size_t, asio::ssl::context_base::password_purpose)
            {
                return pwd;
            });
    }

    auto options = asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2;
    if (!_certificates->temporary_dh.empty()) {
        options |= asio::ssl::context::single_dh_use;
    }

    _ssl_context.set_options(options);

    if (!_certificates->certificat_chain.empty()) {
        _ssl_context.use_certificate_chain(
                asio::buffer(
                        _certificates->certificat_chain.data(),
                        _certificates->certificat_chain.size()));
    }

    if (!_certificates->private_key.empty()) {
        _ssl_context.use_private_key(
                asio::buffer(_certificates->private_key.data(), _certificates->private_key.size()),
                asio::ssl::context::file_format::pem);
    }

    if (!_certificates->temporary_dh.empty()) {
        _ssl_context.use_tmp_dh(
                asio::buffer(_certificates->temporary_dh.data(), _certificates->temporary_dh.size()));
    }

    if (_certificates->certificat_chain.empty() || _certificates->private_key.empty()) {
        _ssl_context.set_verify_mode(asio::ssl::verify_none);
    }
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
#endif

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
wait()
{
    application::Instance().wait();
}

// --------------------------------------------------------------------------
void
stop()
{
    application::Instance().stop();
}

// --------------------------------------------------------------------------
void
post(std::function<void()> fct)
{
    application::Instance().post(std::move(fct));
}

// --------------------------------------------------------------------------
bool is_started()
{
    return application::Instance().is_started();
}

}
