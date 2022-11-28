add_test_executable(
    TEST_NAME server
    SOURCES
        test_long_transaction.cpp
        test_server.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
