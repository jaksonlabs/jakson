cmake_minimum_required(VERSION 3.5)
project(carbon-api-examples-project-setup CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(jakson-test-thread-pool test-thread-pool.cpp)
target_link_libraries(jakson-test-thread-pool PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-jak-result test-jak-result.cpp)
target_link_libraries(jakson-test-jak-result PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

ADD_DEPENDENCIES(tests jakson-test-thread-pool)
ADD_DEPENDENCIES(tests jakson-test-jak-result)

add_test(TestThreadPool ${MAKE_DIRECTORY}/jakson-test-thread-pool)
add_test(TestCarbonResult ${MAKE_DIRECTORY}/jakson-test-jak-result)