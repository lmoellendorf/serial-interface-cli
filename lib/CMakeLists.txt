cmake_minimum_required (VERSION 2.8)
project (serialmac)

# The version number.
set (SERIALMAC_VERSION_MAJOR 0 CACHE STRING "major version")
set (SERIALMAC_VERSION_MINOR 1 CACHE STRING "minor version")
set (SERIALMAC_VERSION_PATCH 0 CACHE STRING "patch version")
set (SERIALMAC_PRODUCT_COMPANY "STACKFORCE" CACHE STRING "company")
set (SERIALMAC_PRODUCT_NAME    "Serial MAC" CACHE STRING "product")


# configure a header file to pass some of the CMake settings
# to the source code
configure_file(${PROJECT_SOURCE_DIR}/version.h.in
               ${PROJECT_BINARY_DIR}/version.h)

list (APPEND SERIALMAC_HEADERS
    ${PROJECT_SOURCE_DIR}/sf_serialmac.h
    ${PROJECT_BINARY_DIR}/version.h
    )

set (SERIALMAC_SOURCES
     ${PROJECT_SOURCE_DIR}/sf_serialmac.c
     ${SERIALMAC_HEADERS}
     )

# add a target to generate API documentation with Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)

    string(CONCAT DOXYFILE_INPUT ${DOXYFILE_INPUT} " "
        ${CMAKE_CURRENT_SOURCE_DIR}/../README.md)
    foreach(loop_var IN LISTS SERIALMAC_HEADERS)
        string(CONCAT DOXYFILE_INPUT ${DOXYFILE_INPUT} " " ${loop_var})
    endforeach(loop_var)

    MESSAGE(STATUS "doxyfile input: ${DOXYFILE_INPUT}")

    # TODO: move doxyfiles below lib or this config to root dir
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in
${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        )
else()
    add_custom_target(doc SOURCES)
endif(DOXYGEN_FOUND)

set(CRC_SOURCE_DIR ../dep/crc)
add_subdirectory(${CRC_SOURCE_DIR} crc)

add_library(serialmac ${SERIALMAC_SOURCES})
add_dependencies(serialmac crc)
include_directories(${CRC_SOURCE_DIR})

install (TARGETS serialmac DESTINATION lib)
install (FILES ${PROJECT_SOURCE_DIR}/sf_serialmac.h DESTINATION include)
