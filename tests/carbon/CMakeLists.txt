cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(test-object-ids test-oid.cpp ${LIB_SOURCES})
target_link_libraries(test-object-ids ${TEST_LIBS})

add_executable(test-archive-ops test-archive-ops.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-ops gtest ${TEST_LIBS})

add_executable(test-fix-map test-fix-map.cpp ${LIB_SOURCES})
target_link_libraries(test-fix-map gtest ${TEST_LIBS})

add_executable(test-archive-iter test-archive-iter.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-iter gtest ${TEST_LIBS})

add_executable(test-archive-converter test-archive-converter.cpp ${LIB_SOURCES})
target_link_libraries(test-archive-converter gtest ${TEST_LIBS})

add_executable(test-varuint test-varuint.cpp ${LIB_SOURCES})
target_link_libraries(test-varuint gtest ${TEST_LIBS})

add_executable(test-bison test-carbon.cpp ${LIB_SOURCES})
target_link_libraries(test-bison gtest ${TEST_LIBS})

add_executable(test-mime test-mime.cpp ${LIB_SOURCES})
target_link_libraries(test-mime gtest ${TEST_LIBS})

ADD_CUSTOM_TARGET(tests)
ADD_DEPENDENCIES(tests test-object-ids)
ADD_DEPENDENCIES(tests test-archive-ops)
ADD_DEPENDENCIES(tests test-fix-map)
ADD_DEPENDENCIES(tests test-archive-iter)
ADD_DEPENDENCIES(tests test-archive-converter)
ADD_DEPENDENCIES(tests test-varuint)
ADD_DEPENDENCIES(tests test-bison)
ADD_DEPENDENCIES(tests test-mime)

add_test(TestObjectIds  ${CMAKE_HOME_DIRECTORY}/build/test-object-ids)
add_test(TestArchiveOps ${CMAKE_HOME_DIRECTORY}/build/test-archive-ops)
add_test(TestFixMap ${CMAKE_HOME_DIRECTORY}/build/test-fix-map)
add_test(TestArchiveIter ${CMAKE_HOME_DIRECTORY}/build/test-archive-iter)
add_test(TestArchiveConverter ${CMAKE_HOME_DIRECTORY}/build/test-archive-converter)
add_test(TestVariableLengthUnsigned ${CMAKE_HOME_DIRECTORY}/build/test-varuint)
add_test(TestBisonFormat ${CMAKE_HOME_DIRECTORY}/build/test-bison)
add_test(TestMimeTypeEncoding ${CMAKE_HOME_DIRECTORY}/build/test-mime)