enable_testing()

find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_INCLUDE_DIRS}/../lib/libcheck.a pthread m)

include_directories(.)

add_executable(tests
        main.c
        ${CMAKE_SOURCE_DIR}/tests/stdx/test_string_hashtable.h
        ${CMAKE_SOURCE_DIR}/tests/stdx/test_string_dic.h
        ${SORUCE_FILES} )

target_link_libraries(tests ${LIBS})

#add_test(stdx_test_string_hashtable test_string_hashtable)