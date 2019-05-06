cmake_minimum_required(VERSION 3.5)
project(tests CXX)

set (CMAKE_CXX_STANDARD 11)

link_directories("/usr/local/lib/")

add_executable(bench-mem-pools EXCLUDE_FROM_ALL mem/pools/main.c ${LIB_SOURCES})
target_link_libraries(bench-mem-pools ${LIBS})

add_executable(bench-index-crack EXCLUDE_FROM_ALL index/crack/main.c ${LIB_SOURCES})
target_link_libraries(bench-index-crack ${LIBS})

ADD_CUSTOM_TARGET(benches)
ADD_DEPENDENCIES(benches bench-mem-pools)
ADD_DEPENDENCIES(benches bench-index-crack)
