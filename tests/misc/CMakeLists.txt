cmake_minimum_required(VERSION 3.5)
project(test-misc CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(jakson-test-thread-pool test-thread-pool.cpp)
target_link_libraries(jakson-test-thread-pool PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

ADD_DEPENDENCIES(tests jakson-test-thread-pool)

add_test(TestThreadPool ${MAKE_DIRECTORY}/jakson-test-thread-pool)