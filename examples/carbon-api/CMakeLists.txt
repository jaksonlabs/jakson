cmake_minimum_required(VERSION 3.9.6)

project(carbon-api-examples)
set (CMAKE_C_STANDARD 11)

ADD_CUSTOM_TARGET(carbon-api-examples)
ADD_DEPENDENCIES(examples carbon-api-examples)

add_subdirectory("${CMAKE_SOURCE_DIR}/examples/carbon-api/project-setup")
add_subdirectory("${CMAKE_SOURCE_DIR}/examples/carbon-api/construct-carbon-records")
add_subdirectory("${CMAKE_SOURCE_DIR}/examples/carbon-api/browse-contents")
