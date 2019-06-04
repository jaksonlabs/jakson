cmake_minimum_required(VERSION 3.5)
project(misc CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(playground EXCLUDE_FROM_ALL playground.c ${LIB_SOURCES})

ADD_CUSTOM_TARGET(misc)
ADD_DEPENDENCIES(misc playground)