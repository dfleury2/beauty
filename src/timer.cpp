#include <beauty/timer.hpp>

#include <iostream>

namespace beauty
{
// --------------------------------------------------------------------------
timer::timer(const beauty::duration& d, timer_cb&& cb, bool repeat) :
    _app(beauty::application::Instance()),
    _timer(_app.ioc()),
    _duration(d),
    _cb(cb),
    _repeat(repeat)
{}

// --------------------------------------------------------------------------
void
timer::run()
{
    _timer.expires_after(_duration);

    _timer.async_wait([me = shared_from_this()](const boost::system::error_code& ec) {
        if (!ec) {
            me->_cb();

            if (me->repeat()) {
                me->run();
            }
        } else {
            std::cout << "Beauty timer expiration issue..." << std::endl;
        }
    });
}


// --------------------------------------------------------------------------
void
after(const duration& d, timer_cb&& cb, bool repeat)
{
    std::make_shared<beauty::timer>(d, std::move(cb), repeat)->run();
}

// --------------------------------------------------------------------------
void after(double seconds, timer_cb&& cb, bool repeat)
{
    after(std::chrono::milliseconds((int)(seconds * 1000)), std::move(cb), repeat);
}

}
