cmake_minimum_required(VERSION 3.5)
project(ark-carbon C)

file(GLOB_RECURSE CMDOPT_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/cmdopt/*.h )
file(GLOB_RECURSE CMDOPT_SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/cmdopt/*.c )
set(CMDOPT_SOURCES ${CMDOPT_HEADER_FILES} ${CMDOPT_SOURCE_FILES})

set(TOOL_SOURCES
        ${LIB_SOURCES}
        ${CMDOPT_SOURCES}
        ${TOOL_SOURCES}
        modules.h
        modules.c)

add_executable(ark-carbon main.c ${TOOL_SOURCES})
target_link_libraries(ark-carbon ${LIBS})

install(TARGETS ark-carbon
        RUNTIME DESTINATION bin)