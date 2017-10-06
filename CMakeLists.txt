cmake_minimum_required(VERSION 3.1)

project(bluezero)

option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TESTS "Build the testcases" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
set(CMAKE_AUTOMOC ON)

find_package(Doxygen)
find_package(Boost 1.54 REQUIRED COMPONENTS thread system regex timer filesystem serialization)
find_package(ZMQ 4.1.4 REQUIRED)
find_package(Protobuf 2.6 REQUIRED)

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
else()
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZMQ_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(
    B0_MSG_SOURCES
    B0_MSG_HEADERS
    protobuf/resolver.proto
    protobuf/logger.proto
)
add_library(b0 SHARED src/b0/node.cpp ${B0_MSG_SOURCES})
target_link_libraries(b0 ${ZMQ_LIBRARY} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY})

add_executable(resolver src/b0/resolver/resolver.cpp src/b0/resolver/resolver_node.cpp)
target_link_libraries(resolver b0)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

