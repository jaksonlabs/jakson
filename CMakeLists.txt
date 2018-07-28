cmake_minimum_required(VERSION 3.5)
project(ng5 C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_C_STANDARD 11)
include_directories("include")
include_directories("/usr/local/include")

set(SORUCE_FILES
        ${CMAKE_SOURCE_DIR}/include/ng5/allocator.h
        ${CMAKE_SOURCE_DIR}/include/status.h
        ${CMAKE_SOURCE_DIR}/include/common.h
        ${CMAKE_SOURCE_DIR}/ng5/allocator.c
        ${CMAKE_SOURCE_DIR}/include/stdx/vector.h
        ${CMAKE_SOURCE_DIR}/stdx/vector.c
        ${CMAKE_SOURCE_DIR}/include/stdx/hash.h
        ${CMAKE_SOURCE_DIR}/include/stdx/hash_table.h
        ${CMAKE_SOURCE_DIR}/include/stdx/slot_map.h
        ${CMAKE_SOURCE_DIR}/include/ng5/storage_engine.h
        ${CMAKE_SOURCE_DIR}/include/ng5/cursor.h
        ${CMAKE_SOURCE_DIR}/include/stdx/asnyc.h
        ${CMAKE_SOURCE_DIR}/stdx/async.c
        ${CMAKE_SOURCE_DIR}/include/ng5/strings.h
        ${CMAKE_SOURCE_DIR}/include/stdx/time.h
        ${CMAKE_SOURCE_DIR}/stdx/time.c
        )

add_executable(ng5 main.c ${SORUCE_FILES})

enable_testing()

add_subdirectory(tests)