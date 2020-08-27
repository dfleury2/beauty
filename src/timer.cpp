#include <beauty/timer.hpp>

namespace beauty
{
// --------------------------------------------------------------------------
void
timer::run()
{
    if (_app.is_stopped()) {
        return;
    }

    if (!_app.is_started()) {
        _app.start();
    }

    _app.timers.push_back(shared_from_this());

    _timer.expires_after(_duration);

    _timer.async_wait([me = shared_from_this()](const boost::system::error_code& ec) {
        if (!ec) {
            if (me->_cb()) {
                me->run();
            } else {
                me->stop();
            }
        }
    });
}

}
