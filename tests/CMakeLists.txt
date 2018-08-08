enable_testing()

find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_INCLUDE_DIRS}/../lib/libcheck.a pthread m)

include_directories(.)

add_executable(test_string_hashtable ${CMAKE_SOURCE_DIR}/tests/stdx/test_string_hashtable.c ${SORUCE_FILES})

target_link_libraries(test_string_hashtable ${LIBS})

add_test(stdx_test_string_hashtable test_string_hashtable)