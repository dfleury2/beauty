#include <doctest/doctest.h>

#include <beauty/timer.hpp>

#include <vector>
#include <iostream>

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
TEST_CASE("After duration timer")
{
    beauty::start();

    bool has_been_called = false;

    auto start = std::chrono::steady_clock::now();
    auto end = start;

    beauty::after(std::chrono::milliseconds(100),
        [&] {
            has_been_called = true;
            end = std::chrono::steady_clock::now();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    CHECK(has_been_called);

    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // Should me 100ms, but allow small variation
    CHECK_GE(delay, 99);
    CHECK_LE(delay, 101);

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("After seconds timer")
{
    beauty::start();

    bool has_been_called = false;
    auto start = std::chrono::steady_clock::now();
    auto end = start;

    beauty::after(0.250, [&] {
        has_been_called = true;
        end = std::chrono::steady_clock::now();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    CHECK(has_been_called);

    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // Should me 1000ms, but allow small variation
    CHECK_GE(delay, 245);
    CHECK_LE(delay, 255);

    beauty::stop();
}
