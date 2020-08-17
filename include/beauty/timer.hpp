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
            _cb = std::move(cb);
        } else {
            _cb = [c = std::move(cb), repeat]() {
                    c();
                    return repeat;
               };
        }
    }

    void run();
    void stop() { _timer.cancel(); }

private:
    beauty::application&    _app;
    asio::steady_timer      _timer;
    duration                _duration;
    timer_cb                _cb;
};

// --------------------------------------------------------------------------
template<typename Callback>
void after(const duration& d, Callback&& cb, bool repeat = false)
{
    std::make_shared<beauty::timer>(d, std::move(cb), repeat)->run();
}

template<typename Callback>
void after(double seconds, Callback&& cb, bool repeat = false)
{
    after(std::chrono::milliseconds((int)(seconds * 1000)), std::move(cb), repeat);
}

template<typename Callback>
void repeat(const duration& d, Callback&& cb)
{
    after(d, std::move(cb), true);
}

template<typename Callback>
void repeat(double seconds, Callback&& cb)
{
    after(seconds, std::move(cb), true);
}

}
