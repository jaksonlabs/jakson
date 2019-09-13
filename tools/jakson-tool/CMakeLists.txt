cmake_minimum_required(VERSION 3.5)
project(jakson-tool C)

set(TOOL_SOURCES
        modules.h
        modules.c)

set(BENCH_SOURCES
        bench/bench_format_handler.h
        bench/bench_format_handler.c
        bench/bench_bson.h
        bench/bench_bson.c
        bench/bench_carbon.h
        bench/bench_carbon.c
        bench/bench_ubjson.h
        bench/bench_ubjson.c
        bench/bench_fwd.h)

add_executable(jakson-tool main.c ${TOOL_SOURCES} ${BENCH_SOURCES})
target_link_libraries(jakson-tool PUBLIC ${LIBS} PRIVATE jakson-sources)

install(TARGETS jakson-tool
        RUNTIME DESTINATION bin)