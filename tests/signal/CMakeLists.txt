add_test_executable(
    TEST_NAME signal
    SOURCES
        test_signal.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
