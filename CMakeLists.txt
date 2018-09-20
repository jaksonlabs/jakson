cmake_minimum_required(VERSION 3.5)
project(ng5 C CXX)

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

include_directories("/usr/local/include")
include_directories(${APRUTIL_INCLUDE_DIR})

set(LIBS ${LIBS} pthread m ${APR_LIBRARIES} ${APRUTIL_LIBRARIES})

aux_source_directory("${CMAKE_SOURCE_DIR}/src" SORUCE_FILES)
aux_source_directory("${CMAKE_SOURCE_DIR}/src" SORUCE_FILES)

add_executable(ng5 main.c ${SORUCE_FILES})

add_executable(compare_cpp benches/baseline/compare_cpp_dic.cpp ${SORUCE_FILES})
set_property(TARGET compare_cpp PROPERTY CXX_STANDARD 11)

target_link_libraries(ng5 ${LIBS})


enable_testing()

add_subdirectory(tests)