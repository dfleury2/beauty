#pragma once

#include <beauty/application.hpp>

#include <boost/asio.hpp>

#include <memory>
#include <functional>

namespace beauty
{
using signal_cb = std::function<void(int signal)>;

// --------------------------------------------------------------------------
class signal_set : public std::enable_shared_from_this<signal_set>
{
public:
    signal_set(signal_cb&& cb);

    signal_set& add(int s);
    void run();

private:
    beauty::application&    _app;
    asio::signal_set        _signals;
    signal_cb               _cb;
};

// --------------------------------------------------------------------------
void signal(int s, signal_cb&& cb);
void signal(std::initializer_list<int>&& signals, signal_cb&& cb);
}
