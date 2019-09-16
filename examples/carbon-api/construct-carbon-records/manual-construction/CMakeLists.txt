cmake_minimum_required(VERSION 3.5)
project(carbon-api-examples-construct-carbon-records CXX)

set (CMAKE_CXX_STANDARD 11)

add_executable(examples-manual-construction manual-construction.c)
target_link_libraries(examples-manual-construction PRIVATE jakson-sources)

add_executable(examples-manual-construction-non-empty manual-construction-non-empty.c)
target_link_libraries(examples-manual-construction-non-empty PRIVATE jakson-sources)

add_executable(examples-manual-construction-arrays manual-construction-arrays.c)
target_link_libraries(examples-manual-construction-arrays PRIVATE jakson-sources)

ADD_DEPENDENCIES(carbon-api-examples examples-manual-construction)
ADD_DEPENDENCIES(carbon-api-examples examples-manual-construction-non-empty)
ADD_DEPENDENCIES(carbon-api-examples examples-manual-construction-arrays)
