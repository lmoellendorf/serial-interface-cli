cmake_minimum_required (VERSION 2.6)
project (libserialport)

# The package version number.
set (SP_PACKAGE_VERSION_MAJOR 0)
set (SP_PACKAGE_VERSION_MINOR 2)
set (SP_PACKAGE_VERSION_MICRO 0)
set (SP_PACKAGE_VERSION
    ${SP_PACKAGE_VERSION_MAJOR}.${SP_PACKAGE_VERSION_MINOR}.${SP_PACKAGE_VERSION_MICRO})

# The lib version number.
set (SP_LIB_VERSION_CURRENT 0)
set (SP_LIB_VERSION_REVISION 0)
set (SP_LIB_VERSION_AGE 0)
set (SP_LIB_VERSION
    ${SP_LIB_VERSION_CURRENT}:${SP_LIB_VERSION_REVISION}:${SP_LIB_VERSION_AGE})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/src/libserialport.h.in"
    "${PROJECT_BINARY_DIR}/libserialport.h"
    )

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -Wall -Wextra -pedantic
-werror")

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/src/" "${PROJECT_BINARY_DIR}/")

set (SERIALPORT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/serialport.c
    )

set (GCC_EXPORT __attribute__\(\(visibility\(\"default\"\)\)\))
set (GCC_PRIVATE __attribute__\(\(visibility\(\"hidden\"\)\)\))
set (WIN_EXPORT __declspec\(dllexport\))
#set (WIN_PRIVATE __declspec\(dllimport\))

# GCC_SUPPORTS_VISIBILITY = true if gcc supports visibility
include (CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fvisibility=hidden GCC_SUPPORTS_VISIBILITY)

if (UNIX AND GCC_SUPPORTS_VISIBILITY)
    MESSAGE(STATUS "GCC with visibility support")
    add_definitions (-DSP_API=${GCC_EXPORT} -DSP_PRIV=${GCC_PRIVATE})
elseif (MSVC)
    MESSAGE(STATUS "MSVC")
    add_definitions (-DSP_API=${WIN_EXPORT} -DSP_PRIV=)
else()
    MESSAGE(STATUS "No visibility support")
    add_definitions (-DSP_API= -DSP_PRIV=)
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    MESSAGE(STATUS "On GNU Linux")
    list (APPEND SERIALPORT_SOURCES
        ${PROJECT_SOURCE_DIR}/src/linux.c
        ${PROJECT_SOURCE_DIR}/src/linux_termios.c
        )
elseif (WIN32)
    MESSAGE(STATUS "On Windows")
    list (APPEND SERIALPORT_SOURCES
        ${PROJECT_SOURCE_DIR}/src/windows.c)
    set (SERIALPORT_TARGET_LINK_LIBS setupapi uuid)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-undefined")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    MESSAGE(STATUS "On Mac OSX")
    list (APPEND SERIALPORT_SOURCES
        ${PROJECT_SOURCE_DIR}/src/macosx.c)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-framework -Wl,IOKit -Wl,-framework -Wl,CoreFoundation")
endif()

# build a shared library
set(BUILD_SHARED_LIBS ON)

add_library(serialport ${SERIALPORT_SOURCES})

target_link_libraries (serialport ${SERIALPORT_TARGET_LINK_LIBS})

#TODO
#library_includedir = $(includedir)
#library_include_HEADERS = libserialport.h
#
#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = libserialport.pc
#
#EXTRA_DIST = Doxyfile
#
#MAINTAINERCLEANFILES = ChangeLog
#
#.PHONY: ChangeLog
#ChangeLog:
#	git --git-dir $(top_srcdir)/.git log > ChangeLog || touch ChangeLog
#
#dist-hook: ChangeLog
#
#doc: $(library_include_HEADERS) $(top_srcdir)/Doxyfile
#	doxygen $(top_srcdir)/Doxyfile
