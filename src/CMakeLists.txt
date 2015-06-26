cmake_minimum_required (VERSION 2.6)
project (serial-interface-lib)

# The version number.
set (SERIAL_INTERFACE_LIB_VERSION_MAJOR 1)
set (SERIAL_INTERFACE_LIB_VERSION_MINOR 0)
set (SERIAL_INTERFACE_LIB_VERSION_PATCH 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(${PROJECT_SOURCE_DIR}/version.h.in
               ${PROJECT_BINARY_DIR}/version.h)

# add the binary tree to the search path for include files
include_directories(${PROJECT_SOURCE_DIR}/../inc/
                    ${PROJECT_SOURCE_DIR}/../lib/crc/)

# build a shared library
set(BUILD_SHARED_LIBS OFF)

list (APPEND SERIAL_INTERFACE_LIB_HEADERS
    ${PROJECT_SOURCE_DIR}/../inc/stackforce_serial_mac_api.h
    ${PROJECT_SOURCE_DIR}/../lib/crc/sf_crc.h
    )

set (SERIAL_INTERFACE_LIB_SOURCES
     ${PROJECT_SOURCE_DIR}/stackforce_serial_mac
     ${PROJECT_SOURCE_DIR}/../lib/crc/sf_crc.c
     ${SERIAL_INTERFACE_LIB_HEADERS}
     )

add_library(serial-interface-lib ${SERIAL_INTERFACE_LIB_SOURCES})

# add a target to generate API documentation with Doxygen

find_package(Doxygen)

if(DOXYGEN_FOUND)

    foreach(loop_var IN LISTS SERIAL_INTERFACE_LIB_HEADERS)
        string(CONCAT DOXYFILE_INPUT ${DOXYFILE_INPUT} " " ${loop_var})
    endforeach(loop_var)
    MESSAGE(STATUS "doxyfile input: ${DOXYFILE_INPUT}")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(doc

        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

        COMMENT "Generating API documentation with Doxygen" VERBATIM

        )

endif(DOXYGEN_FOUND)
