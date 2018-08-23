cmake_minimum_required(VERSION 3.5)
project(ng5 C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/cmake-build-debug)

set (CMAKE_C_STANDARD 11)

file(MAKE_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

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
        ${CMAKE_SOURCE_DIR}/include/stdx/allocator.h
        ${CMAKE_SOURCE_DIR}/include/status.h
        ${CMAKE_SOURCE_DIR}/include/common.h
        ${CMAKE_SOURCE_DIR}/stdx/allocator.c
        ${CMAKE_SOURCE_DIR}/include/stdx/vector.h
        ${CMAKE_SOURCE_DIR}/stdx/vector.c
        ${CMAKE_SOURCE_DIR}/include/stdx/hash.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookup.h
        ${CMAKE_SOURCE_DIR}/include/ng5/storage_engine.h
        ${CMAKE_SOURCE_DIR}/include/stdx/async.h
        ${CMAKE_SOURCE_DIR}/stdx/async.c
        ${CMAKE_SOURCE_DIR}/include/stdx/string_dic.h
        ${CMAKE_SOURCE_DIR}/include/stdx/time.h
        ${CMAKE_SOURCE_DIR}/stdx/time.c
        ${CMAKE_SOURCE_DIR}/include/stdx/unit.h
        ${CMAKE_SOURCE_DIR}/include/stdx/algorithm.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_bsearch.h
        ${CMAKE_SOURCE_DIR}/include/ng5/roadfire/roadfire.h
        ${CMAKE_SOURCE_DIR}/ng5/roadfire/roadfire.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_bsearch.c
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan1.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan1-cache.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan2.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan2-cache.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan3.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan3-cache.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan4.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan4-cache.h
        ${CMAKE_SOURCE_DIR}/include/stdx/string_lookups/simple_scan1-parallel.h
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan1.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan1-cache.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan2.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan2-cache.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan3.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan3-cache.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan4.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan4-cache.c
        ${CMAKE_SOURCE_DIR}/stdx/string_lookups/simple_scan1-parallel.c
        ${CMAKE_SOURCE_DIR}/include/ng5/result_handle.h
        ${CMAKE_SOURCE_DIR}/ng5/result_handle.c
        ${CMAKE_SOURCE_DIR}/include/ng5/roadfire/roadfire.h
        ${CMAKE_SOURCE_DIR}/include/stdx/slot_vector.h
        ${CMAKE_SOURCE_DIR}/stdx/slot_vector.c
        ${CMAKE_SOURCE_DIR}/include/stdx/string_dics/string_dic_async.h
        ${CMAKE_SOURCE_DIR}/stdx/string_dics/string_dic_async.c
        ${CMAKE_SOURCE_DIR}/include/stdx/string_dics/string_dic_naive.h
        ${CMAKE_SOURCE_DIR}/stdx/string_dics/string_dic_naive.c
        ${CMAKE_SOURCE_DIR}/include/stdx/bloomfilter.h
        ${CMAKE_SOURCE_DIR}/ng5/roadfire/roadfire.c
        )


add_executable(ng5 main.c ${SORUCE_FILES})

add_executable(bench-bucket-putget-bsearch benches/bench-bucket-putget-bsearch.c ${SORUCE_FILES} include/stdx/bolster.h include/stdx/slot_vector.h stdx/slot_vector.c)
add_executable(bench-bucket-putget-scan-1 benches/bench-bucket-putget-scan-1.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-2 benches/bench-bucket-putget-scan-2.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-3 benches/bench-bucket-putget-scan-3.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-4 benches/bench-bucket-putget-scan-4.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-1-cache benches/bench-bucket-putget-scan-1-cache.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-2-cache benches/bench-bucket-putget-scan-2-cache.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-3-cache benches/bench-bucket-putget-scan-3-cache.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-4-cache benches/bench-bucket-putget-scan-4-cache.c ${SORUCE_FILES})
add_executable(bench-bucket-putget-scan-1-parallel benches/bench-bucket-putget-scan-1-parallel.c ${SORUCE_FILES})

target_link_libraries(ng5 ${LIBS})
target_link_libraries(bench-bucket-putget-bsearch ${LIBS})
target_link_libraries(bench-bucket-putget-scan-1 ${LIBS})
target_link_libraries(bench-bucket-putget-scan-2 ${LIBS})
target_link_libraries(bench-bucket-putget-scan-3 ${LIBS})
target_link_libraries(bench-bucket-putget-scan-4 ${LIBS})
target_link_libraries(bench-bucket-putget-scan-1-cache ${LIBS})
target_link_libraries(bench-bucket-putget-scan-2-cache ${LIBS})
target_link_libraries(bench-bucket-putget-scan-3-cache ${LIBS})
target_link_libraries(bench-bucket-putget-scan-4-cache ${LIBS})
target_link_libraries(bench-bucket-putget-scan-1-parallel ${LIBS})

enable_testing()

add_subdirectory(tests)