add_test_executable(
    TEST_NAME swagger
    SOURCES
        test_swagger.cpp
    INCLUDES
        ../include
    LIBRARIES
        beauty::beauty
)
