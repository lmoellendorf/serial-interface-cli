cmake_minimum_required (VERSION 2.6)
project (serial-interface)

# The version number.
set (SERIAL_INTERFACE_VERSION_MAJOR 1)
set (SERIAL_INTERFACE_VERSION_MINOR 0)
set (SERIAL_INTERFACE_VERSION_PATCH 0)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic -fmessage-length=0")

if (WIN32)
    MESSAGE(STATUS "On Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
  )

add_subdirectory(${PROJECT_SOURCE_DIR}/../src ${PROJECT_BINARY_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib/libserialport/
    ${PROJECT_BINARY_DIR}/sp_lib)

# add the binary tree to the search path for include files
include_directories(
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/../inc/"
    "${PROJECT_BINARY_DIR}/sp_lib/"
    )

set (SERIAL_INTERFACE_SOURCES
    ${PROJECT_SOURCE_DIR}/main
    ${PROJECT_BINARY_DIR}/version.h
    ${PROJECT_BINARY_DIR}/sp_lib/libserialport.h)

add_executable(stackforce-serial-interface  ${SERIAL_INTERFACE_SOURCES})

# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )
target_link_libraries (stackforce-serial-interface serial-interface-lib
    serialport  ${CMAKE_THREAD_LIBS_INIT})

