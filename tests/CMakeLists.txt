enable_testing()

find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_INCLUDE_DIRS}/../lib/libcheck.a pthread)

include_directories(.)

add_executable(stdx_test_string_id_map ${CMAKE_SOURCE_DIR}/tests/stdx/test_string_id_map.c ${SORUCE_FILES})

target_link_libraries(stdx_test_string_id_map ${LIBS})

add_test(stdx_test_string_id_map stdx_test_string_id_map)