cmake_minimum_required(VERSION 3.9.6)

project(libcarbon VERSION 1.0.0.0 DESCRIPTION "A library to operate with CARBON files"
        LANGUAGES C) # VERSION major.minor.patch.tweak

set (CMAKE_C_STANDARD 11)

#if (${BUILD_TYPE} MATCHES Debug)
#        message("-- Debug build is enabled")
#        set(CMAKE_BUILD_TYPE Debug)
#else()
        message("-- Release build is enabled")
        set(CMAKE_BUILD_TYPE Release)
#endif()

file(MAKE_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}")

include_directories("/usr/include")
include_directories("/usr/local/include")

set(LIBS ${LIBS} pthread m)

file(GLOB_RECURSE LIB_HEADER_FILES src/*.h )
file(GLOB_RECURSE LIB_SOURCE_FILES src/*.c )
file(GLOB_RECURSE LIB_LIBS_HEADER_FILES libs/*.h )
file(GLOB_RECURSE LIB_LIBS_SOURCE_FILES libs/*.c )
set(LIB_SOURCES ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES} ${LIB_LIBS_HEADER_FILES} ${LIB_LIBS_SOURCE_FILES})

if (${LOG_INFO} MATCHES "on")
    if (${BUILD_TYPE} MATCHES Debug)
        message("-- Info log is enabled")
        add_definitions(-DARK_LOG_INFO)
    else()
        message("-- ** WARNING ** Info log is enabled but ignored")
        message("   use `cmake -DBUILD_TYPE=Debug` to turn on logging")
    endif()
endif()
if (${LOG_TRACE} MATCHES "on")
    if (${BUILD_TYPE} MATCHES Debug)
        message("-- Trace log is enabled")
        add_definitions(-DARK_LOG_TRACE)
    else()
        message("-- ** WARNING ** Trace log is enabled but ignored")
        message("   use `cmake -DBUILD_TYPE=Debug` to turn on logging")
    endif()
endif()
if (${LOG_DEBUG} MATCHES "on")
    if (${BUILD_TYPE} MATCHES Debug)
        message("-- Debug log is enabled")
        add_definitions(-DARK_LOG_DEBUG)
    else()
        message("-- ** WARNING ** Debug log is enabled but ignored")
        message("   use `cmake -DBUILD_TYPE=Debug` to turn on logging")
    endif()
endif()
if (${LOG_WARN} MATCHES "on")
    if (${BUILD_TYPE} MATCHES Debug)
        message("-- Warn log is enabled")
        add_definitions(-DARK_LOG_DEBUG)
    else()
        message("-- ** WARNING ** Warn log is enabled but ignored")
        message("   use `cmake -DBUILD_TYPE=Debug` to turn on logging")
    endif()
endif()

set (CMAKE_C_FLAGS                "-std=c11 -Wall -Wextra -Werror")
set (CMAKE_C_FLAGS_DEBUG          "-g")
set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_CXX_FLAGS                "-std=c++11 -Wall -Wextra -Werror")
set (CMAKE_CXX_FLAGS_DEBUG          "-g")
set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

add_library(ark-sources STATIC ${LIB_SOURCES}) # used to share object files across different dependent targets
link_libraries(ark-sources ${LIBS})

add_subdirectory("${CMAKE_SOURCE_DIR}/tools")

enable_testing()
add_subdirectory("${CMAKE_SOURCE_DIR}/tests")


