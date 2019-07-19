cmake_minimum_required (VERSION 2.6)
project (BSONPP)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 11)
option(BUILD_TESTS "Build all tests." OFF)

add_definitions(-D__LINUX_BUILD)

include_directories(src)

set(SRCS src/BSONPP.cpp)

# Build the shared library
add_library(BSONPP_shared SHARED ${SRCS})
set_target_properties(BSONPP_shared PROPERTIES OUTPUT_NAME "bsonpp")
target_include_directories(BSONPP_shared PUBLIC src)

# Build the static library
add_library(BSONPP_static STATIC ${SRCS})
set_target_properties(BSONPP_static PROPERTIES OUTPUT_NAME "bsonpp")
target_include_directories(BSONPP_static PUBLIC src)

if (BUILD_TESTS)
add_subdirectory(googletest)

enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} .)

add_executable(${PROJECT_NAME}_Test test/Test.cpp)
target_link_libraries(${PROJECT_NAME}_Test gtest gtest_main BSONPP_static)
endif()
