cmake_minimum_required(VERSION 3.9.6)

project(tests)
set (CMAKE_C_STANDARD 11)

ADD_CUSTOM_TARGET(tests)

add_subdirectory("${CMAKE_SOURCE_DIR}/tests/carbon")
add_subdirectory("${CMAKE_SOURCE_DIR}/tests/misc")
