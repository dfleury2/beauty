function(add_test_executable)
    set(prefix ARG)
    set(singleValues TEST_NAME)
    set(multiValues INCLUDES SOURCES LIBRARIES)

    cmake_parse_arguments("${prefix}" "" "${singleValues}" "${multiValues}" ${ARGN})

    set(TARGET_NAME tst_${ARG_TEST_NAME})

    add_executable(${TARGET_NAME} ${ARG_SOURCES})

    add_test(NAME "${ARG_TEST_NAME}.t" COMMAND ${TARGET_NAME})

    add_dependencies(tests ${TARGET_NAME})

    if (ARG_INCLUDES)
        target_include_directories(${TARGET_NAME} PRIVATE ${ARG_INCLUDES})
    endif()

    if (ARG_LIBRARIES)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_LIBRARIES})
    endif()

    set_target_properties(${TARGET_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/t
        OUTPUT_NAME "${ARG_TEST_NAME}.t"
    )

endfunction()
