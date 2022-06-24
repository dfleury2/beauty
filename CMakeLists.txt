cmake_minimum_required(VERSION 3.15)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

project(Beauty)
set(VERSION 1.0.0-rc2)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
if (NOT DEFINED CONAN OR CONAN)
	find_package(Boost CONFIG REQUIRED)
	find_package(OpenSSL CONFIG REQUIRED)
else()
	find_package(Boost REQUIRED)
	find_package(OpenSSL REQUIRED)

    add_library(openssl::openssl ALIAS OpenSSL::SSL)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
