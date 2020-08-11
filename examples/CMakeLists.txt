# yes, I know...
file(GLOB SOURCES *.cpp)

foreach(source ${SOURCES})
    get_filename_component(filename ${source} NAME)
    get_filename_component(targetname ${source} NAME_WE)

    add_executable(${targetname} ${filename})
    
    target_link_libraries(${targetname} beauty::beauty)
endforeach()
