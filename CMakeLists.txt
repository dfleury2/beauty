cmake_minimum_required(VERSION 3.21)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
set(VERSION 1.0.4)

project(Beauty LANGUAGES CXX VERSION ${VERSION})

option(BEAUTY_ENABLE_OPENSSL "Enable OpenSSL support" OFF)
option(BEAUTY_BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTING "Build tests" ON)

find_package(Boost CONFIG REQUIRED)
if (BEAUTY_ENABLE_OPENSSL)
    find_package(OpenSSL CONFIG REQUIRED)
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)

if (BEAUTY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_TESTING)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
