#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/application.hpp>
#include <beauty/timer.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Run and stop")
{
    beauty::application app;

    CHECK_FALSE(app.is_started());

    app.start();

    CHECK(app.is_started());

    app.stop();

    CHECK_FALSE(app.is_started());
}

// --------------------------------------------------------------------------
TEST_CASE("Run Twice")
{
    beauty::start();
    CHECK(beauty::is_started());

    bool timer_is_called = false;
    beauty::after(0.100, [&] {timer_is_called = true;});
    beauty::after(0.150, [&] {timer_is_called = false;});

    beauty::start();

    // The first timer is called
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    CHECK(timer_is_called);

    // The second timer must not be called
    beauty::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(60));

    CHECK(timer_is_called);
}

// --------------------------------------------------------------------------
TEST_CASE("Asynchronous post")
{
    beauty::application app;
    app.start();

    int was_called = 0;
    auto f = [&was_called]() { ++was_called; };

    app.post(f);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    app.stop();

    CHECK_EQ(was_called, 1);
}
