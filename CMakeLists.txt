cmake_minimum_required(VERSION 3.15)

project(Beauty)
set(VERSION 0.1-rc)

if (CONAN_EXPORTED)
else()
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
        include(${PROJECT_SOURCE_DIR}/cmake/conan.cmake)

        conan_cmake_autodetect(settings)

        conan_cmake_install(PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}/conanfile.py
                PROFILE ${CONAN_PROFILE}
                PROFILE_BUILD ${CONAN_PROFILE}
                BUILD missing
                SETTINGS ${settings}
                )
    endif()
endif()

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
find_package(Boost CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
add_subdirectory(examples)
