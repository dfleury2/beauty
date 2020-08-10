add_library(beauty
    ../include/beauty/acceptor.hpp
    ../include/beauty/application.hpp
    ../include/beauty/attributes.hpp
    ../include/beauty/beauty.hpp
    ../include/beauty/certificate.hpp
    ../include/beauty/client.hpp
    ../include/beauty/header.hpp
    ../include/beauty/request.hpp
    ../include/beauty/response.hpp
    ../include/beauty/route.hpp
    ../include/beauty/router.hpp
    ../include/beauty/server.hpp
    ../include/beauty/acceptor.hpp
    ../include/beauty/session.hpp
    ../include/beauty/signal.hpp
    ../include/beauty/timer.hpp
    ../include/beauty/url.hpp
    ../include/beauty/utils.hpp
    ./acceptor.cpp
    ./application.cpp
    ./attributes.cpp
    ./client.cpp
    ./route.cpp
    ./server.cpp
    ./signal.cpp
    ./timer.cpp
    ./url.cpp
    ./utils.cpp
)

add_library(beauty::beauty ALIAS beauty)

set_target_properties(beauty
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_STANDARD 20
        LINK_FLAGS "-Wl,-z,relro -Wl,--as-needed"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        OUTPUT_NAME beauty
        SOVERSION ${VERSION}
        VERSION ${VERSION}
)

target_compile_definitions(beauty
    PRIVATE
        BOOST_ALL_DYN_LINK
        LINUX
        UNIX
)

target_include_directories(beauty
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(beauty
    PUBLIC
        OpenSSL::SSL
        Boost::system
        Threads::Threads
)

configure_file(./version.hpp.in ./beauty/version.hpp)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include
    DESTINATION include
)

install(TARGETS beauty
    EXPORT BeautyConfig
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
)

install(EXPORT BeautyConfig
    FILE BeautyConfig.cmake
    NAMESPACE beauty::
    DESTINATION lib/cmake/beauty
)
