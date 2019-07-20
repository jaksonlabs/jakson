cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated-includes")

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/test-constants.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated-includes/test-constants.h"
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/generated-includes")

add_executable(test-object-ids EXCLUDE_FROM_ALL test-oid.cpp ${LIB_SOURCES})
target_link_libraries(test-object-ids ${TEST_LIBS})

add_executable(test-archive-ops EXCLUDE_FROM_ALL test-archive-ops.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-ops gtest ${TEST_LIBS})

add_executable(test-fix-map EXCLUDE_FROM_ALL test-fix-map.cpp ${LIB_SOURCES})
target_link_libraries(test-fix-map gtest ${TEST_LIBS})

add_executable(test-archive-iter EXCLUDE_FROM_ALL test-archive-iter.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-iter gtest ${TEST_LIBS})

add_executable(test-archive-converter EXCLUDE_FROM_ALL test-archive-converter.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-converter gtest ${TEST_LIBS})

add_executable(test-auto-compressor EXCLUDE_FROM_ALL test-auto-compressor.cpp ${LIB_SOURCES})
target_link_libraries(test-auto-compressor gtest ${TEST_LIBS})

add_executable(test-rule-selector EXCLUDE_FROM_ALL test-rule-selector.cpp ${LIB_SOURCES})
target_link_libraries(test-rule-selector gtest ${TEST_LIBS})

ADD_CUSTOM_TARGET(tests)
ADD_DEPENDENCIES(tests test-object-ids)
ADD_DEPENDENCIES(tests test-archive-ops)
ADD_DEPENDENCIES(tests test-fix-map)
ADD_DEPENDENCIES(tests test-archive-iter)
ADD_DEPENDENCIES(tests test-archive-converter)
ADD_DEPENDENCIES(tests test-auto-compressor)
ADD_DEPENDENCIES(tests test-rule-selector)

add_test(TestObjectIds  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-object-ids)
add_test(TestArchiveOps ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-archive-ops)
add_test(TestFixMap ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-fix-map)
add_test(TestArchiveIter ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-archive-iter)
add_test(TestArchiveConverter ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-archive-converter)
add_test(TestAutoCompressor ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-auto-compressor)
add_test(TestRuleSelector ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-rule-selector)
