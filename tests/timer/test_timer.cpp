#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/timer.hpp>

#include <vector>

using namespace std::chrono_literals;

// --------------------------------------------------------------------------
namespace std {
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    for(size_t i = 0; i < v.size(); ++i) {
        os << (i ? ", " : "") << v[i];
    }
    return os << "]";
}

}

// --------------------------------------------------------------------------
TEST_CASE("Simple timer")
{
    beauty::start();

    bool has_been_called = false;

    auto start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;

    std::make_shared<beauty::timer>(100ms,
            [&] {
                has_been_called = true;
                end = std::chrono::steady_clock::now();
            })->start();

    std::this_thread::sleep_for(150ms);

    CHECK(has_been_called);

    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // Should me 100ms, but allow small variation
    CHECK_GE(delay, 90);
    CHECK_LE(delay, 110);

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Simple timer multiple time")
{
    beauty::start();

    int call_count = 0;

    std::make_shared<beauty::timer>(50ms,
            [&] {
                ++call_count;
            }, true)->start();

    std::this_thread::sleep_for(230ms);

    CHECK_EQ(call_count, 4);

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Two timers")
{
    beauty::start();

    std::vector<int> v;

    std::make_shared<beauty::timer>(30ms,
            [&] {
                v.push_back(1);
            })->start();

    std::make_shared<beauty::timer>(130ms,
            [&] {
                v.push_back(2);
            })->start();

    std::this_thread::sleep_for(150ms);

    CHECK_EQ(v, std::vector<int>{1, 2});

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Nested timers")
{
    beauty::start();

    std::vector<int> v;

    SUBCASE("Using timer") {
        std::make_shared<beauty::timer>(50ms,
                [&v] {
                    v.push_back(1);

                    std::make_shared<beauty::timer>(80ms,
                            [&v] {
                                v.push_back(2);
                            })->start();

                })->start();

        std::this_thread::sleep_for(60ms);
        CHECK_EQ(v, std::vector<int>{1});

        std::this_thread::sleep_for(90ms);
        CHECK_EQ(v, std::vector<int>{1, 2});
    }

    SUBCASE("Using beauty::after") {
        beauty::after(50ms,
                [&v] {
                    v.push_back(1);

                    beauty::after(80ms,
                            [&v] {
                                v.push_back(2);
                            });

                });

        std::this_thread::sleep_for(60ms);
        CHECK_EQ(v, std::vector<int>{1});

        std::this_thread::sleep_for(90ms);
        CHECK_EQ(v, std::vector<int>{1, 2});
    }

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Stopped timers")
{
    beauty::start();

    // Start a repeat each 50ms
    int call_count = 0;
    auto t1 = beauty::repeat(50ms, [&call_count] { ++call_count; });

    // Wait for 2 repeat to trigger
    std::this_thread::sleep_for(125ms);
    CHECK_EQ(call_count, 2);

    // Stop the repeat
    t1->stop();

    // Wait another 2 theoretical repeat
    std::this_thread::sleep_for(100ms);
    CHECK_EQ(call_count, 2); // No increment, since stopped

    // Restart the repeat
    t1->start();

    // Wait another 2 repeat
    std::this_thread::sleep_for(130ms);
    CHECK_EQ(call_count, 4);

    // Double stop
    t1->stop();
    t1->stop();

    std::this_thread::sleep_for(100ms);
    CHECK_EQ(call_count, 4);

    // The timer is only present once in the app timers
    CHECK_EQ(beauty::application::Instance().timers.size(), 1);

    beauty::stop();
}
