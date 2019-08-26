cmake_minimum_required(VERSION 3.5)
project(jakson-tool C)

set(TOOL_SOURCES
        modules.h
        modules.c)

add_executable(jakson-tool main.c ${TOOL_SOURCES})
target_link_libraries(jakson-tool PUBLIC ${LIBS} PRIVATE jakson-sources)

install(TARGETS jakson-tool
        RUNTIME DESTINATION bin)