add_test_executable(
    TEST_NAME timer
    SOURCES
        test_after.cpp
        test_repeat.cpp
        test_timer.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
