cmake_minimum_required(VERSION 3.5)
project(bison-examples CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

add_executable(doc-create-string-list ${LIB_SOURCES} doc-create-string-list.cpp)
add_executable(doc-create-number-column ${LIB_SOURCES} doc-create-number-column.cpp)

set_target_properties(doc-create-string-list PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(doc-create-number-column PROPERTIES EXCLUDE_FROM_ALL TRUE)

ADD_DEPENDENCIES(examples doc-create-string-list)
ADD_DEPENDENCIES(examples doc-create-number-column)
