cmake_minimum_required(VERSION 3.5)
project(ark-tools C)

add_subdirectory("${CMAKE_SOURCE_DIR}/tools/jakson-tool")

ADD_CUSTOM_TARGET(tools)
ADD_DEPENDENCIES(tools jakson-tool)