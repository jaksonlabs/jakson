cmake_minimum_required(VERSION 3.5)
project(test-carbon CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

add_executable(jakson-test-object-ids test-global_id.cpp)
target_link_libraries(jakson-test-object-ids PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-archive-ops test-archive-ops.cpp)
target_link_libraries(jakson-test-archive-ops PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-fix-map test-fix-map.cpp)
target_link_libraries(jakson-test-fix-map PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-archive-iter test-archive-iter.cpp)
target_link_libraries(jakson-test-archive-iter PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-archive-converter test-archive-converter.cpp)
target_link_libraries(jakson-test-archive-converter PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-varuint test-varuint.cpp)
target_link_libraries(jakson-test-varuint PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-carbon test-carbon.cpp)
target_link_libraries(jakson-test-carbon PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-mime test-mime.cpp)
target_link_libraries(jakson-test-mime PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-json test-json.cpp)
target_link_libraries(jakson-test-json PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

add_executable(jakson-test-abstract-type-markers test-abstract-type-markers.cpp)
target_link_libraries(jakson-test-abstract-type-markers PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)

ADD_DEPENDENCIES(tests jakson-test-object-ids)
ADD_DEPENDENCIES(tests jakson-test-archive-ops)
ADD_DEPENDENCIES(tests jakson-test-fix-map)
ADD_DEPENDENCIES(tests jakson-test-archive-iter)
ADD_DEPENDENCIES(tests jakson-test-archive-converter)
ADD_DEPENDENCIES(tests jakson-test-varuint)
ADD_DEPENDENCIES(tests jakson-test-carbon)
ADD_DEPENDENCIES(tests jakson-test-mime)
ADD_DEPENDENCIES(tests jakson-test-json)
ADD_DEPENDENCIES(tests jakson-test-abstract-type-markers)

add_test(TestObjectIds  ${MAKE_DIRECTORY}/jakson-test-object-ids)
add_test(TestArchiveOps ${MAKE_DIRECTORY}/jakson-test-archive-ops)
add_test(TestFixMap ${MAKE_DIRECTORY}/jakson-test-fix-map)
add_test(TestArchiveIter ${MAKE_DIRECTORY}/jakson-test-archive-iter)
add_test(TestArchiveConverter ${MAKE_DIRECTORY}/jakson-test-archive-converter)
add_test(TestVariableLengthUnsigned ${MAKE_DIRECTORY}/jakson-test-varuint)
add_test(TestCarbonFiles ${MAKE_DIRECTORY}/jakson-test-carbon)
add_test(TestMimeTypeEncoding ${MAKE_DIRECTORY}/jakson-test-mime)
add_test(TestJson ${MAKE_DIRECTORY}/jakson-test-json)
add_test(TestJson ${MAKE_DIRECTORY}/jakson-test-abstract-type-markers)