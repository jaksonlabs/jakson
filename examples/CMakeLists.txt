cmake_minimum_required(VERSION 3.9.6)

project(examples)
set (CMAKE_C_STANDARD 11)

ADD_CUSTOM_TARGET(examples)

add_subdirectory("${CMAKE_SOURCE_DIR}/examples/carbon-api")
