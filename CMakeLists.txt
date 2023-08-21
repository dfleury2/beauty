cmake_minimum_required(VERSION 3.21)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

project(Beauty)
set(VERSION 1.0.1-rc)

# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
if (NOT DEFINED CONAN OR CONAN)
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
        message(FATAL_ERROR "In Conan mode, you must run a 'conan install' command")
    endif()

    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

    find_package(Boost CONFIG REQUIRED)
    if (BEAUTY_ENABLE_OPENSSL)
        find_package(OpenSSL CONFIG REQUIRED)
    endif()
else()
    find_package(Boost REQUIRED)

    if (BEAUTY_ENABLE_OPENSSL)
        find_package(OpenSSL REQUIRED)
        add_library(openssl::openssl ALIAS OpenSSL::SSL)
    endif()
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)

if (NOT CONAN_IN_LOCAL_CACHE)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
