cmake_minimum_required (VERSION 2.6)
project (serial-interface)

# The version number.
set (SERIAL_INTERFACE_VERSION_MAJOR 1)
set (SERIAL_INTERFACE_VERSION_MINOR 0)
set (SERIAL_INTERFACE_VERSION_PATCH 0)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
  )

link_directories(${PROJECT_SOURCE_DIR}/../ide/eclipse/libserialport/src/.libs/)

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../inc/")
include_directories("${PROJECT_SOURCE_DIR}/../ide/eclipse/libserialport/src/")

add_subdirectory(${PROJECT_SOURCE_DIR}/../src ${PROJECT_BINARY_DIR}/lib)

add_executable(stackforce-serial-interface ${PROJECT_SOURCE_DIR}/main)

target_link_libraries (stackforce-serial-interface serial-interface-lib
    serialport pthread)

