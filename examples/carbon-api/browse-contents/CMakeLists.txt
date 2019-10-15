cmake_minimum_required(VERSION 3.5)
project(carbon-api-examples-project-setup CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(examples-read-record-contents read-record-contents.c)
target_link_libraries(examples-read-record-contents PRIVATE jakson-sources)

ADD_DEPENDENCIES(carbon-api-examples examples-read-record-contents)
