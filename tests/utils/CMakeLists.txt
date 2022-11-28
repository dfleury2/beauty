add_test_executable(
    TEST_NAME utils
    SOURCES
        test_utils.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
