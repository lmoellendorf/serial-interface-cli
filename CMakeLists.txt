cmake_minimum_required (VERSION 2.6)
project (serial-interface)

# The version number.
set (serial-interface_VERSION_MAJOR 1)
set (serial-interface_VERSION_MINOR 0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -lpthread")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/app/serial-interfaceConfig.h.in"
  "${PROJECT_BINARY_DIR}/serial-interfaceConfig.h"
  )

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/ide/eclipse/libserialport/src/.libs/)

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/inc/")
include_directories("${PROJECT_SOURCE_DIR}/ide/eclipse/libserialport/src/")

add_subdirectory(src)

add_executable(stackforce-serial-interface ${PROJECT_SOURCE_DIR}/app/main)

target_link_libraries (stackforce-serial-interface serial-interface-lib
    serialport pthread)

