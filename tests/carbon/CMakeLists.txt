cmake_minimum_required(VERSION 3.5)
project(test-carbon CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

set(
    TEST_LIBS
        gtest
        ${LIBS}
)

macro(CreateTest arg)
    add_executable(${arg} ${arg}.cpp)
    target_link_libraries(${arg} PUBLIC gtest ${TEST_LIBS} PRIVATE jakson-sources)
    ADD_DEPENDENCIES(tests ${arg})
    add_test(${arg}  ${MAKE_DIRECTORY}/${arg})
endmacro()

CreateTest(test-global_id)
CreateTest(test-archive-ops)
CreateTest(test-fix-map)
CreateTest(test-archive-iter)
CreateTest(test-archive-converter)
CreateTest(test-varuint)
CreateTest(test-mime)
CreateTest(test-json)
CreateTest(test-carbon)
CreateTest(test-abstract-type-markers)
CreateTest(test-abstract-types-arrays)
CreateTest(test-abstract-types-columns)
CreateTest(test-abstract-types-objects)
CreateTest(test-carbon-patch)
