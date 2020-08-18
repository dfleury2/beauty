# yes, I know...
file(GLOB SOURCES *.cpp)

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
