cmake_minimum_required (VERSION 2.6)
project (serial-interface-lib)

## The version number.
#set (serial-interface-lib_VERSION_MAJOR 1)
#set (serial-interface-lib_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#  "${PROJECT_SOURCE_DIR}/app/serial-interfaceConfig.h.in"
#  "${PROJECT_BINARY_DIR}/serial-interfaceConfig.h"
#  )

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/../inc/")

add_library(serial-interface-lib ${PROJECT_SOURCE_DIR}/stackforce_serial_mac)
