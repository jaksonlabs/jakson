cmake_minimum_required(VERSION 3.5)
project(carbon-api-examples-project-setup CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(examples-hello-carbon hello-carbon.c)
target_link_libraries(examples-hello-carbon PRIVATE jakson-sources)

ADD_DEPENDENCIES(carbon-api-examples examples-hello-carbon)
