#include <beauty/timer.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
void
timer::start()
{
    check_application();

    register_timer();

    rearm();
}

// --------------------------------------------------------------------------
void
timer::check_application()
{
    if (_app.is_stopped()) {
        return;
    }

    if (!_app.is_started()) {
        _app.start();
    }
}

// --------------------------------------------------------------------------
void
timer::register_timer()
{
    bool found = false;
    for (auto& t : _app.timers) {
        if (t.get() == this) {
            found = true;
            break;
        }
    }
    if (!found) {
        _app.timers.push_back(shared_from_this());
    }
}

// --------------------------------------------------------------------------
void
timer::rearm()
{
    _timer.expires_after(_duration);

    _timer.async_wait([me = shared_from_this()](const boost::system::error_code& ec) {
        if (!ec) {
            if (me->_cb()) {
                me->rearm();
            }
        }
    });
}

}
