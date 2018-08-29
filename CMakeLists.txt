cmake_minimum_required(VERSION 3.5)
project(ng5 C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/cmake-build-debug)

set (CMAKE_C_STANDARD 11)

file(MAKE_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)

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

include(${CMAKE_SOURCE_DIR}/cmake/modules/FindAPR.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/modules/FindCheck.cmake)

include_directories("include")
include_directories("/usr/local/include")
include_directories(${APRUTIL_INCLUDE_DIR})

set(LIBS ${LIBS} pthread m ${APR_LIBRARIES} ${APRUTIL_LIBRARIES})

set(SORUCE_FILES
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_allocator.h
        ${CMAKE_SOURCE_DIR}/include/ng5_status.h
        ${CMAKE_SOURCE_DIR}/include/ng5_common.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_allocator.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_vector.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_vector.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_hash.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_string_map.h
        ${CMAKE_SOURCE_DIR}/include/ng5/ng5_storage_engine.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_spinlock.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_spinlock.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_string_dic.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_time.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_time.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_unit.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_algorithm.h
        ${CMAKE_SOURCE_DIR}/include/ng5/roadfire/ng5_roadfire.h
        ${CMAKE_SOURCE_DIR}/ng5/roadfire/ng5_roadfire.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_string_map_smart.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_string_map_smart.c
        ${CMAKE_SOURCE_DIR}/include/ng5/ng5_result_handle.h
        ${CMAKE_SOURCE_DIR}/ng5/ng5_result_handle.c
        ${CMAKE_SOURCE_DIR}/include/ng5/roadfire/ng5_roadfire.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_slot_vector.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_slot_vector.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_string_dic_async.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_string_dic_async.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_string_dic_sync.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_string_dic_sync.c
        ${CMAKE_SOURCE_DIR}/ng5/roadfire/ng5_roadfire.c
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_bolster.h
        ${CMAKE_SOURCE_DIR}/include/stdx/ng5_trace_alloc.h
        ${CMAKE_SOURCE_DIR}/stdx/ng5_trace_alloc.c
        ${CMAKE_SOURCE_DIR}/include/ng5_static_conf.h)


add_executable(ng5 main.c ${SORUCE_FILES})

target_link_libraries(ng5 ${LIBS})


enable_testing()

add_subdirectory(tests)