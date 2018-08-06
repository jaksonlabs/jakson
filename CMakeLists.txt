cmake_minimum_required(VERSION 3.5)
project(ng5 C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set (CMAKE_C_STANDARD 11)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set (CMAKE_C_COMPILER             "/usr/bin/clang")
set (CMAKE_C_FLAGS                "-Wall -std=c11 -Wextra -Werror")
set (CMAKE_C_FLAGS_DEBUG          "-g")
set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set (CMAKE_CXX_FLAGS                "-std=c++11 -Wall")
set (CMAKE_CXX_FLAGS_DEBUG          "-g")
set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_AR      "/usr/bin/llvm-ar")
set (CMAKE_LINKER  "/usr/bin/llvm-ld")
set (CMAKE_NM      "/usr/bin/llvm-nm")
set (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
set (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

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
        ${CMAKE_SOURCE_DIR}/include/stdx/string_hashtable.h
        ${CMAKE_SOURCE_DIR}/include/stdx/slot_map.h
        ${CMAKE_SOURCE_DIR}/include/ng5/storage_engine.h
        ${CMAKE_SOURCE_DIR}/include/ng5/cursor.h
        ${CMAKE_SOURCE_DIR}/include/stdx/asnyc.h
        ${CMAKE_SOURCE_DIR}/stdx/async.c
        ${CMAKE_SOURCE_DIR}/include/ng5/strings.h
        ${CMAKE_SOURCE_DIR}/include/stdx/time.h
        ${CMAKE_SOURCE_DIR}/stdx/time.c
        ${CMAKE_SOURCE_DIR}/ng5/strings.c
        ${CMAKE_SOURCE_DIR}/include/stdx/unit.h
        ${CMAKE_SOURCE_DIR}/include/stdx/algorithm.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_id_maps/simple_bsearch.h
        ${CMAKE_SOURCE_DIR}/stdx/string_id_maps/simple_bsearch.c
)

add_executable(ng5 main.c ${SORUCE_FILES})

enable_testing()

add_subdirectory(tests)