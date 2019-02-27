cmake_minimum_required(VERSION 3.5)
project(carbontools C)

file(GLOB_RECURSE CMDOPT_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/cmdopt/*.h )
file(GLOB_RECURSE CMDOPT_SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/cmdopt/*.c )
set(CMDOPT_SOURCES ${CMDOPT_HEADER_FILES} ${CMDOPT_SOURCE_FILES})

set(TOOL_SOURCES
        ${LIB_SOURCES}
        ${CMDOPT_SOURCES}
        ${TOOL_SOURCES}
        modules.h
        modules.c cli.h cli.c)

add_executable(carbon-tool main.c ${TOOL_SOURCES})
target_link_libraries(carbon-tool ${LIBS})
set_target_properties(carbon-tool PROPERTIES EXCLUDE_FROM_ALL TRUE)