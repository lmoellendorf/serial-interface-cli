cmake_minimum_required (VERSION 2.6)
project (serialinterface)

# The version number.
set (SERIALINTERFACE_VERSION_MAJOR 1)
set (SERIALINTERFACE_VERSION_MINOR 0)
set (SERIALINTERFACE_VERSION_PATCH 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(${PROJECT_SOURCE_DIR}/version.h.in
               ${PROJECT_BINARY_DIR}/version.h)

# add the binary tree to the search path for include files
include_directories(${PROJECT_SOURCE_DIR}/../inc/
                    ${PROJECT_SOURCE_DIR}/../lib/crc/)

# build a shared library
set(BUILD_SHARED_LIBS OFF)

list (APPEND SERIALINTERFACE_HEADERS
    ${PROJECT_SOURCE_DIR}/../inc/sf_serialmac.h
    ${PROJECT_SOURCE_DIR}/../lib/crc/sf_crc.h
    )

set (SERIALINTERFACE_SOURCES
     ${PROJECT_SOURCE_DIR}/sf_serialmac.c
     ${PROJECT_SOURCE_DIR}/../lib/crc/sf_crc.c
     ${SERIALINTERFACE_HEADERS}
     )

add_library(serialinterface ${SERIALINTERFACE_SOURCES})

# add a target to generate API documentation with Doxygen

find_package(Doxygen)

if(DOXYGEN_FOUND)

    foreach(loop_var IN LISTS SERIALINTERFACE_HEADERS)
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

install (TARGETS serialinterface DESTINATION lib)
install (FILES ${PROJECT_SOURCE_DIR}/../inc/sf_serialmac.h DESTINATION include)
