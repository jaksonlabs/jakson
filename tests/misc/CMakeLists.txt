cmake_minimum_required(VERSION 3.5)
project(test-misc CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(test-thread-pool EXCLUDE_FROM_ALL test-thread-pool.cpp ${LIB_SOURCES})
target_link_libraries(test-thread-pool gtest ${TEST_LIBS})

ADD_DEPENDENCIES(tests test-thread-pool)

add_test(TestThreadPool ${CMAKE_HOME_DIRECTORY}/build/test-thread-pool)