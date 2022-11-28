add_test_executable(
    TEST_NAME url
    SOURCES
        test_url.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
