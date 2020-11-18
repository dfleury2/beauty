#include <beauty/application.hpp>

#include <beauty/certificate.hpp>
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
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _state(State::waiting)
{}

// --------------------------------------------------------------------------
application::application(certificates&& c) :
    _work(asio::make_work_guard(_ioc)),
    _ssl_context(asio::ssl::context::sslv23),
    _certificates(std::move(c)),
    _state(State::waiting)
{
    load_server_certificates();
}

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
    if (is_started()) {
        return;
    }

    if (is_stopped()) {
        // The application was started before, we need
        // to restart the ioc cleanly
        _ioc.restart();
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
                    _ioc.run();
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
    if (is_stopped()) {
        return;
    }
    _state = State::stopped;

    if (reset) {
        for(auto&& t : timers) {
            t->stop();
        }
        timers.clear();
    }

    _ioc.stop();

    while(_active_threads != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// --------------------------------------------------------------------------
void
application::run()
{
    if (is_stopped()) {
        _ioc.restart();
    }
    _state = State::started;

    // Run
    _ioc.run();
}

// --------------------------------------------------------------------------
void
application::wait()
{
    while(!is_stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

// --------------------------------------------------------------------------
void
application::post(std::function<void()> fct)
{
    boost::asio::post(_ioc.get_executor(), std::move(fct));
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
