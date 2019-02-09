cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)

link_directories("/usr/local/lib/")

add_executable(test-object-ids test-oid.cpp ${LIB_SOURCES})
target_link_libraries(test-object-ids gtest)

message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME testObjectId COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-object-ids)