cmake_minimum_required (VERSION 2.6)
project (serial-interface-lib)

# The version number.
set (SERIAL_INTERFACE_LIB_VERSION_MAJOR 1)
set (SERIAL_INTERFACE_LIB_VERSION_MINOR 0)
set (SERIAL_INTERFACE_LIB_VERSION_PATCH 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
    )

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/../inc/")

# build a shared library
set(BUILD_SHARED_LIBS OFF)

add_library(serial-interface-lib ${PROJECT_SOURCE_DIR}/stackforce_serial_mac)
