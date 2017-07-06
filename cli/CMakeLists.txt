## @code
##  ___ _____ _   ___ _  _____ ___  ___  ___ ___
## / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
## \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
## |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
## embedded.connectivity.solutions.==============
## @endcode
##
## @file
## @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
## @author     Lars Möllendorf
## @author     Adrian Antonana
## @brief      STACKFORCE Serial MAC Command Line Client
##
## @details See @code sf --help @endcode for details.
##
## This file is part of the STACKFORCE Serial Command Line Client
## (below "serialmac cli").
##
## The serialmac cli is free software: you can redistribute it and/or modify
## it under the terms of the GNU Affero General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## The serialmac cli is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Affero General Public License for more details.
##
## You should have received a copy of the GNU Affero General Public License
## along with libserialmac.  If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required (VERSION 3.2)
project (serialcli)

include(ExternalProject)

##
## TARGET
##
set(SF_SERIALMACCLI_SOURCES
    ${PROJECT_SOURCE_DIR}/main
    ${PROJECT_SOURCE_DIR}/sf_serialmaccli.cpp
    ${PROJECT_SOURCE_DIR}/sf_serialmachandler.cpp
    ${PROJECT_SOURCE_DIR}/sf_stringhex.cpp
)

add_executable(sf ${SF_SERIALMACCLI_SOURCES})

add_dependencies(
    sf
    docopt
    serialport
    serialmac
    observer
)

if(CMAKE_CROSSCOMPILING AND WIN32)
    add_dependencies(
        sf
        getstdthreads
    )
endif()

target_include_directories(sf PRIVATE
    ${PROJECT_SOURCE_DIR}
)

##
## PRODUCT AND VERSION
##
set (SERIALMACCLI_PRODUCT_NAME "sf-serial" CACHE STRING "product")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
)

# add the binary tree to the search path for generated include files
target_include_directories(sf PUBLIC ${PROJECT_BINARY_DIR})

##
## TOOLCHAIN OPTIONS
##
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -fmessage-length=0")

if(WIN32)
    if(CMAKE_CROSSCOMPILING)
        message(STATUS "Cross compiling for Windows")
        message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
        add_definitions(-D__WIN32_CROSS_BUILD__)
    else()
        message(STATUS "On Windows")
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
    # Not all compilers offer macros to indicate a Windows build environment.
    add_definitions(-D__WIN32__)

    # Find the setupapi library
    find_library (SETUPAPI setupapi)
    if(SETUPAPI)
        message(STATUS "Setupapi library location: ${SETUPAPI}")
    else()
        message(FATAL_ERROR "Failed to locate the setupapi library")
    endif()
    set(WIN32_DEPS ${SETUPAPI})
endif()

##
## EXTERNAL PROJECTS
##
# DOCOPT
set(DOCOPT_CMAKE_EXTRA_ARGS)
if(CMAKE_CROSSCOMPILING)
    list(APPEND DOCOPT_CMAKE_EXTRA_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()

if(WIN32)
    list(APPEND DOCOPT_CMAKE_EXTRA_ARGS "-DWITH_STATIC=true")
endif()

ExternalProject_Add(libdocopt   # Name for custom target
    #--Download step--------------
    GIT_REPOSITORY https://github.com/docopt/docopt.cpp.git
    GIT_TAG a4177ccf1a6e36ebe268972732ddd456a3574f6d
    #--Update step----------------
    # Never update automatically from the remote repository
    UPDATE_DISCONNECTED 1
    #--Configure step-----------------
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )

externalproject_get_property(libdocopt install_dir)
include_directories("${install_dir}/include/docopt")
if(WIN32)
    add_library(docopt IMPORTED STATIC GLOBAL)
    # Set docopt properties
    set_target_properties(docopt PROPERTIES
        "IMPORTED_LOCATION" "${install_dir}/lib/libdocopt_s.a"
    )
else()
    add_library(docopt IMPORTED SHARED GLOBAL)
    set_target_properties(docopt PROPERTIES
        "IMPORTED_LOCATION" "${install_dir}/lib/libdocopt.so"
    )
endif()

add_dependencies(docopt libdocopt)

# SERIALPORT
set(LIBSERIALPORT_INSTALL_DIR ${PROJECT_BINARY_DIR}/libserialport-prefix)
find_library(LIBSERIALPORT serialport ${LIBSERIALPORT_INSTALL_DIR}/lib/)

set(LIBSERIALPORT_CONFIGURE_PARAMETER)

if(CMAKE_CROSSCOMPILING)
    list(APPEND LIBSERIALPORT_CONFIGURE_PARAMETER --host=${COMPILER_PREFIX})
endif()

if(WIN32)
    list(APPEND LIBSERIALPORT_CONFIGURE_PARAMETER --disable-shared)
endif()

if(NOT LIBSERIALPORT)
    message(STATUS "libserialport not found!")
    ExternalProject_Add(libserialport    # Name for custom target
        #--Download step--------------
        GIT_REPOSITORY git://sigrok.org/libserialport
        GIT_TAG ae2c5825e79ab5b38a8ca53941df6d73679a52e6
        #--Update step----------------
        # Never update automatically from the remote repository
        UPDATE_DISCONNECTED 1
        #--Configure step-------------
        # set prefix to the default install dir
        # (libserialport-prefix in this case)
        CONFIGURE_COMMAND ./autogen.sh && ./configure ${LIBSERIALPORT_CONFIGURE_PARAMETER} --prefix=<INSTALL_DIR>
        #--Build step-----------------
        BUILD_COMMAND make
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 0
        #--Install step-----------------
        INSTALL_COMMAND make install
    )

    ExternalProject_Get_Property(libserialport
        install_dir build_command configure_command
    )

    message(STATUS "libserialport will be installed to:\n "
        ${install_dir} "\nusing build command: " ${build_command}
        "\nusing configure command: " ${configure_command}
    )

    # re-run cmake in initial build
    # will update cmakecache/project files on first build
    # so you may have to reload project after first build
    add_custom_target(serialport
        COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS libserialport
    )
else()
    message(STATUS "libserialport found!")
    # serialport becomes a dummy target after first build
    # this prevents cmake from rebuilding cache/projects on subsequent builds
    add_custom_target(serialport)
endif()

set(LIBSERIALPORT_HEADERS ${LIBSERIALPORT_INSTALL_DIR}/include/libserialport.h)
set_source_files_properties(${LIBSERIALPORT_HEADERS} PROPERTIES GENERATED TRUE)
include_directories(${LIBSERIALPORT_INSTALL_DIR}/include)

# mingw-std-threads
if(CMAKE_CROSSCOMPILING AND WIN32)
    add_custom_target(getstdthreads
        COMMAND git clone -b master https://github.com/meganz/mingw-std-threads.git
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    target_include_directories(sf PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/mingw-std-threads)
    message(STATUS "mingw-std-threads location: ${CMAKE_CURRENT_BINARY_DIR}/mingw-std-threads")
endif()

##
## INTERNAL PROJECTS
##
# SERIALMAC
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib lib)
# OBSERVER
add_subdirectory(${PROJECT_SOURCE_DIR}/../dep/observer observer)

##
## LINKING
##
# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package(Threads REQUIRED)

if(LIBSERIALPORT) # This is needed for integrating the non-CMake project

    target_link_libraries(sf
        serialmac
        ${LIBSERIALPORT}
        docopt
        observer
        ${CMAKE_THREAD_LIBS_INIT}
        ${WIN32_DEPS}
    )
endif()

##
## INSTALLATION
##
install(TARGETS sf DESTINATION bin)

###
### TESTING
###
include(${PROJECT_SOURCE_DIR}/../CMakeIncludeGTest.txt)

# stringhex
add_executable(teststringhex
    test/test.cpp
    test/sf_teststringhex.cpp
    sf_stringhex.cpp
    )
target_include_directories(teststringhex PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(teststringhex libgmock)
add_test(NAME teststringhex COMMAND teststringhex)

