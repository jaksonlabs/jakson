cmake_minimum_required(VERSION 3.5)
project(jakson-tool C)

file(GLOB_RECURSE CMDOPT_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/cmdopt/*.h )
file(GLOB_RECURSE CMDOPT_SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/cmdopt/*.c )
set(CMDOPT_SOURCES ${CMDOPT_HEADER_FILES} ${CMDOPT_SOURCE_FILES})

set(TOOL_SOURCES
        ${CMDOPT_SOURCES}
        ${TOOL_SOURCES}
        modules.h
        modules.c ../../src/jak_utils_numbers.c)

add_executable(jakson-tool main.c ${TOOL_SOURCES})
target_link_libraries(jakson-tool PUBLIC ${LIBS} PRIVATE jakson-sources)

install(TARGETS jakson-tool
        RUNTIME DESTINATION bin)