cmake_minimum_required(VERSION 3.6)

project(Beauty)
set(VERSION 0.1-rc)

# Conan setup
include(${CMAKE_SOURCE_DIR}/cmake/conan.cmake)

conan_cmake_run(CONANFILE conanfile.py
        PROFILE ${CONAN_PROFILE}
        BASIC_SETUP CMAKE_TARGETS)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
