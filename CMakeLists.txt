cmake_minimum_required(VERSION 3.6)

project(Beauty)
set(VERSION 0.1-rc)

option(CONAN "Enable/disable Conan" ON)

if (${CONAN})
    include(${CMAKE_SOURCE_DIR}/cmake/conan.cmake)

    conan_cmake_run(CONANFILE conanfile.py
            PROFILE ${CONAN_PROFILE}
            BASIC_SETUP CMAKE_TARGETS)

    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
# You should take care about which FindBoost.cmake is used...
find_package(Boost REQUIRED COMPONENTS system)

# Thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# OpenSSl
find_package(OpenSSL)

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
