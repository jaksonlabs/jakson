cmake_minimum_required(VERSION 3.5)
project(test-carbon CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(test-object-ids test-global_id.cpp)
target_link_libraries(test-object-ids PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-archive-ops test-archive-ops.cpp)
target_link_libraries(test-archive-ops PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-fix-map test-fix-map.cpp)
target_link_libraries(test-fix-map PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-archive-iter test-archive-iter.cpp)
target_link_libraries(test-archive-iter PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-archive-converter test-archive-converter.cpp)
target_link_libraries(test-archive-converter PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-varuint test-varuint.cpp)
target_link_libraries(test-varuint PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-carbon test-carbon.cpp)
target_link_libraries(test-carbon PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-mime test-mime.cpp)
target_link_libraries(test-mime PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

add_executable(test-json test-json.cpp)
target_link_libraries(test-json PUBLIC gtest ${TEST_LIBS} PRIVATE ark-sources)

ADD_DEPENDENCIES(tests test-object-ids)
ADD_DEPENDENCIES(tests test-archive-ops)
ADD_DEPENDENCIES(tests test-fix-map)
ADD_DEPENDENCIES(tests test-archive-iter)
ADD_DEPENDENCIES(tests test-archive-converter)
ADD_DEPENDENCIES(tests test-varuint)
ADD_DEPENDENCIES(tests test-carbon)
ADD_DEPENDENCIES(tests test-mime)
ADD_DEPENDENCIES(tests test-json)

add_test(TestObjectIds  ${CMAKE_HOME_DIRECTORY}/build/test-object-ids)
add_test(TestArchiveOps ${CMAKE_HOME_DIRECTORY}/build/test-archive-ops)
add_test(TestFixMap ${CMAKE_HOME_DIRECTORY}/build/test-fix-map)
add_test(TestArchiveIter ${CMAKE_HOME_DIRECTORY}/build/test-archive-iter)
add_test(TestArchiveConverter ${CMAKE_HOME_DIRECTORY}/build/test-archive-converter)
add_test(TestVariableLengthUnsigned ${CMAKE_HOME_DIRECTORY}/build/test-varuint)
add_test(TestCarbonFiles ${CMAKE_HOME_DIRECTORY}/build/test-carbon)
add_test(TestMimeTypeEncoding ${CMAKE_HOME_DIRECTORY}/build/test-mime)
add_test(TestJson ${CMAKE_HOME_DIRECTORY}/build/test-json)