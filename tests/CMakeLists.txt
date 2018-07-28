enable_testing()

find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_INCLUDE_DIRS}/../lib/libcheck.a)

include_directories(.)

add_executable(stdx_test_slot_map stdx/test_slot_map.c ${SORUCE_FILES})

target_link_libraries(stdx_test_slot_map ${LIBS})

add_test(stdx_test_slot_map stdx_test_slot_map)