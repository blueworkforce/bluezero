cmake_minimum_required(VERSION 3.1)

project(bluezero)

include(CheckIncludeFileCXX)

option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TESTS "Build the testcases" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # gcc
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # visual c++
endif()
set(CMAKE_AUTOMOC ON)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DBOOST_USE_WINDOWS_H)
endif()

find_package(Doxygen)
find_package(Boost 1.54 REQUIRED COMPONENTS thread system regex timer filesystem serialization)
find_package(ZMQ 4.1.4 REQUIRED)
find_package(Protobuf 2.6 REQUIRED)
find_package(ZLIB)
find_package(LZ4)

if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    set(DOCS_IN ${CMAKE_CURRENT_BINARY_DIR}/docs/html)
    set(DOCS_OUT ${CMAKE_CURRENT_SOURCE_DIR}/docs)

    add_custom_command(TARGET doc POST_BUILD
        COMMAND rm -rf ${DOCS_OUT}
        COMMAND cp -r ${DOCS_IN} ${DOCS_OUT}
        COMMENT "Copying HTML docs to docs/ dir"
        VERBATIM)
else()
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/cppzmq/zmq.hpp)
    execute_process(COMMAND git submodule init WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    execute_process(COMMAND git submodule update WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/cppzmq)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZMQ_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIR})

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_have_boost_process/main.cpp "#include <boost/process.hpp>
int main(){return 0;}")
try_compile(HAVE_BOOST_PROCESS
    ${CMAKE_CURRENT_BINARY_DIR}/test_have_boost_process
    ${CMAKE_CURRENT_BINARY_DIR}/test_have_boost_process/main.cpp
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${Boost_INCLUDE_DIRS}
    COMPILE_DEFINITIONS "${Boost_DEFINITIONS} -std=c++11"
    LINK_LIBRARIES ${Boost_LIBRARIES}
    COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/test_have_boost_process/main)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/b0/config.h)

protobuf_generate_cpp(
    B0_MSG_SOURCES
    B0_MSG_HEADERS
    protobuf/envelope.proto
    protobuf/resolver.proto
    protobuf/logger.proto
)
add_library(
    b0
    SHARED
    src/b0/node.cpp
    src/b0/publisher.cpp
    src/b0/service_client.cpp
    src/b0/service_server.cpp
    src/b0/subscriber.cpp
    src/b0/compress/compress.cpp
    src/b0/compress/lz4.cpp
    src/b0/compress/zlib.cpp
    src/b0/exception/exception.cpp
    src/b0/exception/invalid_state_transition.cpp
    src/b0/exception/message_unpack_error.cpp
    src/b0/exception/unsupported_compression_algorithm.cpp
    src/b0/logger/logger.cpp
    src/b0/resolver/client.cpp
    src/b0/resolver/resolver.cpp
    src/b0/socket/socket.cpp
    src/b0/utils/thread_name.cpp
    ${B0_MSG_SOURCES}
)
target_link_libraries(b0 ${ZMQ_LIBRARY} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY})
if(WIN32)
    target_link_libraries(b0 wsock32 ws2_32)
endif()

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(b0 ${ZLIB_LIBRARIES})
endif()
if(LZ4_FOUND)
    include_directories(${LZ4_INCLUDE_DIR})
    target_link_libraries(b0 ${LZ4_LIBRARY})
endif()

add_executable(resolver src/b0/resolver/resolver.cpp src/b0/resolver/resolver_main.cpp)
target_link_libraries(resolver b0)

add_executable(logger_console src/b0/logger/console.cpp)
target_link_libraries(logger_console b0)

add_executable(graph_console src/b0/graph/console.cpp)
target_link_libraries(graph_console b0)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

