cmake_minimum_required(VERSION 3.15)

project(Beauty)
set(VERSION 1.0.0-rc2)

# Check if conan is disabled
if (NOT DEFINED CONAN OR CONAN)
    if (NOT DEFINED CONAN_IN_LOCAL_CACHE OR NOT CONAN_IN_LOCAL_CACHE)
        include(${PROJECT_SOURCE_DIR}/cmake/conan.cmake)

        conan_cmake_autodetect(settings)

        conan_cmake_install(PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}/conanfile.py
            PROFILE ${CONAN_PROFILE}
            PROFILE_BUILD ${CONAN_PROFILE}
            BUILD missing
            SETTINGS ${settings}
            OUTPUT_FOLDER ${CMAKE_BINARY_DIR}
        )
    endif()

    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

# Conan declare openssl::openssl alias and Cmake create OpenSSL::SSL
if(NOT TARGET openssl::openssl)
    add_library(openssl::openssl ALIAS OpenSSL::SSL)
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
