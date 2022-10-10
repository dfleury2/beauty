#pragma once

#include <beauty/application.hpp>

#include <boost/asio.hpp>

#include <memory>
#include <functional>

namespace beauty
{
using duration = std::chrono::steady_clock::duration;
using timer_cb = std::function<bool()>;

// --------------------------------------------------------------------------
class timer : public std::enable_shared_from_this<timer>
{
public:
    template<typename Callback>
    timer(const beauty::duration& d, Callback&& cb, bool repeat = false) :
        _app(beauty::application::Instance()),
        _timer(_app.ioc()),
        _duration(d)
    {
        if constexpr(std::is_same_v<decltype(cb()), bool>) {
            _cb = std::forward<Callback>(cb);
        } else {
            _cb = [c = std::forward<Callback>(cb), repeat]() {
                    c();
                    return repeat;
               };
        }
    }

    void start();
    void stop() { _timer.cancel(); }

private:
    void check_application();
    void register_timer();
    void rearm();

private:
    beauty::application&    _app;
    asio::steady_timer      _timer;
    duration                _duration;
    timer_cb                _cb;
};

// --------------------------------------------------------------------------
template<typename Callback>
std::shared_ptr<beauty::timer>
after(const duration& d, Callback&& cb, bool repeat = false)
{
    auto timer = std::make_shared<beauty::timer>(d, std::forward<Callback>(cb), repeat);
    timer->start();
    return timer;
}

template<typename Callback>
std::shared_ptr<beauty::timer>
after(double seconds, Callback&& cb, bool repeat = false)
{
    return after(std::chrono::milliseconds((int)(seconds * 1000)), std::forward<Callback>(cb), repeat);
}

template<typename Callback>
std::shared_ptr<beauty::timer>
repeat(const duration& d, Callback&& cb)
{
    return after(d, std::forward<Callback>(cb), true);
}

template<typename Callback>
std::shared_ptr<beauty::timer>
repeat(double seconds, Callback&& cb)
{
    return after(seconds, std::forward<Callback>(cb), true);
}

}
