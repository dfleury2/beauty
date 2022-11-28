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
TEST_CASE("Repeat timer")
{
    beauty::start();

    int called = 0;

    beauty::repeat(std::chrono::milliseconds(40),
        [&] {
            ++called;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(130));

    CHECK_EQ(called, 3);

    beauty::stop();
}

// --------------------------------------------------------------------------
TEST_CASE("Repeat timer but stop")
{
    beauty::start();

    int call = 0;

    beauty::repeat(std::chrono::milliseconds(40),
        [&] {
            ++call;
            if (call == 2) {
                return false;
            }
            return true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(160));

    CHECK_EQ(call, 2);

    beauty::stop();
}
