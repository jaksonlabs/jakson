cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(test-object-ids EXCLUDE_FROM_ALL test-oid.cpp ${LIB_SOURCES})
target_link_libraries(test-object-ids ${TEST_LIBS})

add_executable(test-archive-ops EXCLUDE_FROM_ALL test-archive-ops.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-ops gtest ${TEST_LIBS})

add_executable(test-fix-map EXCLUDE_FROM_ALL test-fix-map.cpp ${LIB_SOURCES})
target_link_libraries(test-fix-map gtest ${TEST_LIBS})

add_executable(test-archive-iter EXCLUDE_FROM_ALL test-archive-iter.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-iter gtest ${TEST_LIBS})

add_executable(test-archive-visitor EXCLUDE_FROM_ALL test-archive-visitor.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-visitor gtest ${TEST_LIBS})

ADD_CUSTOM_TARGET(tests)
ADD_DEPENDENCIES(tests test-object-ids)
ADD_DEPENDENCIES(tests test-archive-ops)
ADD_DEPENDENCIES(tests test-fix-map)
ADD_DEPENDENCIES(tests test-archive-iter)
ADD_DEPENDENCIES(tests test-archive-visitor)

add_test(TestObjectIds  ${CMAKE_HOME_DIRECTORY}/build/test-object-ids)
add_test(TestArchiveOps ${CMAKE_HOME_DIRECTORY}/build/test-archive-ops)
add_test(TestFixMap ${CMAKE_HOME_DIRECTORY}/build/test-fix-map)
add_test(TestArchiveIter ${CMAKE_HOME_DIRECTORY}/build/test-archive-iter)
add_test(TestArchiveVisitor ${CMAKE_HOME_DIRECTORY}/build/test-archive-visitor)