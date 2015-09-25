cmake_minimum_required (VERSION 2.8)
project (observer)

## The version number.
#set (OBSERVER_VERSION_MAJOR 0 CACHE STRING "major version")
#set (OBSERVER_VERSION_MINOR 1 CACHE STRING "minor version")
#set (OBSERVER_VERSION_PATCH 0 CACHE STRING "patch version")
#set (OBSERVER_PRODUCT_COMPANY "STACKFORCE" CACHE STRING "company")
#set (OBSERVER_PRODUCT_NAME    "Observer Pattern Implementation" CACHE STRING "product")
#
#
## configure a header file to pass some of the CMake settings
## to the source code
#configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in
#    ${PROJECT_BINARY_DIR}/version.h)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic -fmessage-length=0")

# add the binary tree to the search path for include files
include_directories(${PROJECT_BINARY_DIR}/
    ${PROJECT_SOURCE_DIR}/)

list (APPEND OBSERVER_HEADERS
    ${PROJECT_SOURCE_DIR}/sf_observer.h
    ${PROJECT_SOURCE_DIR}/sf_subject.h
    ${PROJECT_SOURCE_DIR}/sf_event.h
    #    ${PROJECT_BINARY_DIR}/version.h
    )

set (OBSERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/sf_subject.cpp
    ${PROJECT_SOURCE_DIR}/sf_event.cpp
    ${OBSERVER_HEADERS}
    )

## add a target to generate API documentation with Doxygen
#find_package(Doxygen)
#
#if(DOXYGEN_FOUND)
#
#    string(CONCAT DOXYFILE_INPUT ${DOXYFILE_INPUT} " "
#        ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
#    foreach(loop_var IN LISTS OBSERVER_HEADERS)
#        string(CONCAT DOXYFILE_INPUT ${DOXYFILE_INPUT} " " ${loop_var})
#    endforeach(loop_var)
#
#    MESSAGE(STATUS "doxyfile input: ${DOXYFILE_INPUT}")
#
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
#        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
#
#    add_custom_target(doc
#        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#        COMMENT "Generating API documentation with Doxygen" VERBATIM
#        SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
#        )
#else()
#    add_custom_target(doc SOURCES)
#endif(DOXYGEN_FOUND)
#
##list(APPEND OBSERVER_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_library(observer ${OBSERVER_SOURCES})

install (TARGETS observer DESTINATION lib)
install (FILES ${PROJECT_SOURCE_DIR}/sf_observer.h
    ${PROJECT_SOURCE_DIR}/sf_subject.h
    DESTINATION include)
