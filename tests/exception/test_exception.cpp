#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <beauty/exception.hpp>

// --------------------------------------------------------------------------
TEST_CASE("Empty exception")
{
    beauty::exception ex;
}
