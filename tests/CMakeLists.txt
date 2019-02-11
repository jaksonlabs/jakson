cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

add_executable(test-object-ids EXCLUDE_FROM_ALL test-oid.cpp ${LIB_SOURCES})
target_link_libraries(test-object-ids gtest)

add_executable(test-archive-ops EXCLUDE_FROM_ALL test-archive-ops.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-ops gtest)

ADD_CUSTOM_TARGET(tests)
ADD_DEPENDENCIES(tests test-object-ids)
ADD_DEPENDENCIES(tests test-archive-ops)

add_test(TestObjectIds  ${CMAKE_HOME_DIRECTORY}/build/test-object-ids)
add_test(TestArchiveOps ${CMAKE_HOME_DIRECTORY}/build/test-archive-ops)