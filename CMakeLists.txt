cmake_minimum_required(VERSION 3.15)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

project(Beauty)
set(VERSION 1.0.0-rc2)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
find_package(Boost CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)

# Conan declare openssl::openssl alias and Cmake create OpenSSL::SSL
if(NOT TARGET openssl::openssl)
    message(STATUS "openssl::openssl not defined, using OpenSSL::SSL as fallback")
    add_library(openssl::openssl ALIAS OpenSSL::SSL)
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
