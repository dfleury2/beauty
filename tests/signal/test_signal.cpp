#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/signal.hpp>

// --------------------------------------------------------------------------
TEST_CASE("One signal")
{
#if LINUX
    beauty::start();

    bool has_been_called = false;

    beauty::signal(SIGUSR1,
        [&](int signal) {
            has_been_called = !has_been_called;
            CHECK_EQ(signal, SIGUSR1);
    });

    CHECK_FALSE(has_been_called);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ::kill(getpid(), SIGUSR1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK(has_been_called);

    ::kill(getpid(), SIGUSR1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_FALSE(has_been_called);

    beauty::stop();
#endif
}

// --------------------------------------------------------------------------
TEST_CASE("Two signal on one signal set")
{
#if LINUX
    beauty::start();

    bool sigusr1_has_been_called = false;
    bool sigusr2_has_been_called = false;

    beauty::signal({SIGUSR1, SIGUSR2},
        [&](int signal) {
            switch(signal) {
            case SIGUSR1: sigusr1_has_been_called = true; break;
            case SIGUSR2: sigusr2_has_been_called = true; break;
            }
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ::kill(getpid(), SIGUSR1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ::kill(getpid(), SIGUSR2);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK(sigusr1_has_been_called);
    CHECK(sigusr2_has_been_called);

    beauty::stop();
#endif
}

// --------------------------------------------------------------------------
TEST_CASE("One signal on two signal set")
{
#if LINUX
    beauty::start();

    bool sigusr1_has_been_called = false;
    bool sigusr2_has_been_called = false;

    beauty::signal(SIGUSR1, [&](int signal) { sigusr1_has_been_called = !sigusr1_has_been_called; });
    beauty::signal(SIGUSR2, [&](int signal) { sigusr2_has_been_called = !sigusr2_has_been_called; });

    SUBCASE("First signal") {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::kill(getpid(), SIGUSR1);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::kill(getpid(), SIGUSR2);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        CHECK(sigusr1_has_been_called);
        CHECK(sigusr2_has_been_called);
    }

    SUBCASE("Second signal") {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::kill(getpid(), SIGUSR1);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::kill(getpid(), SIGUSR2);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        CHECK_FALSE(sigusr1_has_been_called);
        CHECK_FALSE(sigusr2_has_been_called);
    }

    beauty::stop();
#endif
}
