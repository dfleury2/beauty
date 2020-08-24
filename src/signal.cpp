#include <beauty/signal.hpp>

#include <iostream>

namespace beauty
{
// --------------------------------------------------------------------------
signal_set::signal_set(signal_cb&& cb) :
    _app(beauty::application::Instance()),
    _signals(_app.ioc()),
    _cb(cb)
{}

// --------------------------------------------------------------------------
signal_set&
signal_set::add(int s)
{
    _signals.add(s);
    return *this;
}

// --------------------------------------------------------------------------
void
signal_set::run()
{
    if (_app.is_stopped()) {
        return;
    }

    if (!_app.is_started()) {
        _app.start();
    }

    _signals.async_wait([me = shared_from_this()](const boost::system::error_code& ec, int s) {
        if (!ec) {
            me->_cb(s);

            // Keep the signal active
            me->run();
        } else {
            std::cout << "Beauty signal issue..." << std::endl;
        }
    });
}


// --------------------------------------------------------------------------
void
signal(std::initializer_list<int>&& signals, signal_cb&& cb)
{
    auto signal_set = std::make_shared<beauty::signal_set>(std::move(cb));
    for(auto&& s: signals) {
        signal_set->add(s);
    }
    signal_set->run();
}

// --------------------------------------------------------------------------
void
signal(int s, signal_cb&& cb)
{
    signal({s}, std::move(cb));
}

}
