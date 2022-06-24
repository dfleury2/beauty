set(SOURCES
    beauty_application.cpp
    beauty_application_singleton.cpp
    beauty_benchmark.cpp
    beauty_chat.cpp
    beauty_client_async.cpp
    beauty_client_sync.cpp
    beauty_io_context_server.cpp
    beauty_server.cpp
    beauty_server_attributes.cpp
    beauty_server_postpone.cpp
    beauty_server_singleton.cpp
    beauty_simple_server.cpp
    beauty_ws_client.cpp
    client_async.cpp
    client_sync.cpp
    server.cpp
    server_long_transaction.cpp
    signal.cpp
)

if(BEAUTY_ENABLE_OPENSSL)
    list(APPEND SOURCES
        beauty_client_async_ssl.cpp
        beauty_client_sync_ssl.cpp
        beauty_server_ssl.cpp
        beauty_simple_ssl_server.cpp
    )
endif()


foreach(source ${SOURCES})
    get_filename_component(filename ${source} NAME)
    get_filename_component(targetname ${source} NAME_WE)

    add_executable(${targetname} ${filename})

    target_compile_definitions(${targetname}
      PRIVATE
        BOOST_ALL_STATIC_LINK=1
        BOOST_ASIO_DISABLE_BOOST_ARRAY=1
        BOOST_ASIO_DISABLE_BOOST_BIND=1
        BOOST_ASIO_DISABLE_BOOST_DATE_TIME=1
        BOOST_ASIO_DISABLE_BOOST_REGEX=1
        # Disable a bug in Asio 1.73 with gcc 10.1 and C++20
        BOOST_ASIO_DISABLE_CONCEPTS=1
        BOOST_ASIO_NO_DEPRECATED=1
#        BOOST_ASIO_SEPARATE_COMPILATION=1
#        BOOST_BEAST_SEPARATE_COMPILATION=1
        BOOST_COROUTINES_NO_DEPRECATION_WARNING=1
        )

    target_link_libraries(${targetname} beauty::beauty)

    if(WIN32)
        set_target_properties(${targetname}
            PROPERTIES
                LINK_FLAGS "/ignore:4099"
        )
    endif()
endforeach()
