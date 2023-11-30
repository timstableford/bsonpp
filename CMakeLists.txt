cmake_minimum_required (VERSION 3.15)
project (bsonpp)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 11)
option(BUILD_TESTS "Build all tests." OFF)

add_definitions(-D__LINUX_BUILD)

include_directories(src)

set(SRCS src/BSONPP.cpp)

# Build the shared library
add_library(bsonpp_shared SHARED ${SRCS})
set_target_properties(bsonpp_shared PROPERTIES OUTPUT_NAME "bsonpp")
target_include_directories(bsonpp_shared PUBLIC src)

# Build the static library
add_library(bsonpp STATIC ${SRCS})
set_target_properties(bsonpp PROPERTIES OUTPUT_NAME "bsonpp")
target_include_directories(bsonpp
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS bsonpp
        EXPORT bsonpp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

add_library(bsonpp::bsonpp ALIAS bsonpp)

include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/bsonpp
    CACHE PATH "Location of header files" )
set(SYSCONFIG_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}/bsonpp
    CACHE PATH "Location of configuration files" )
#...
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/bsonpp-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/bsonpp-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bsonpp
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/bsonpp-config-version.cmake
  VERSION 1.2.3
  COMPATIBILITY SameMajorVersion )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bsonpp-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/bsonpp-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bsonpp )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/BSONPP.h 
              ${CMAKE_CURRENT_SOURCE_DIR}/src/IEEE754tools.h 
              ${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkUtil.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bsonpp)

if (BUILD_TESTS)
  add_subdirectory(googletest)

  enable_testing()

  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} .)

  add_executable(${PROJECT_NAME}_Test test/Test.cpp)
  target_link_libraries(${PROJECT_NAME}_Test gtest gtest_main bsonpp)
endif()
