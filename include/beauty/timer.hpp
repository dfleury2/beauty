#pragma once

#include <beauty/application.hpp>

#include <boost/asio.hpp>

#include <memory>
#include <functional>

namespace beauty
{
using duration = std::chrono::steady_clock::duration;
using timer_cb = std::function<void()>;

// --------------------------------------------------------------------------
class timer : public std::enable_shared_from_this<timer>
{
public:
    timer(const duration&, timer_cb&& cb, bool repeat = false);

    void run();

    void repeat(bool v) { _repeat = v; }
    bool repeat() const { return _repeat; }

private:
    beauty::application&    _app;
    asio::steady_timer      _timer;
    duration                _duration;
    timer_cb                _cb;
    bool                    _repeat = false;
};

// --------------------------------------------------------------------------
void after(const duration& d, timer_cb&& cb, bool repeat = false);
void after(double seconds, timer_cb&& cb, bool repeat = false);

void repeat(const duration& d, timer_cb&& cb);
void repeat(double seconds, timer_cb&& cb);

}
