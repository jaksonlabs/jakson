cmake_minimum_required(VERSION 3.5)
project(carbon-api-examples-construct-carbon-records CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(examples-from-json from-json.c)
target_link_libraries(examples-from-json PRIVATE jakson-sources)

ADD_DEPENDENCIES(carbon-api-examples examples-from-json)
