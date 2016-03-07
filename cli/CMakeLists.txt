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
## @author     STACKFORCE
## @author     Lars Möllendorf
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
    )

add_executable(sf  ${SF_SERIALMACCLI_SOURCES})

add_dependencies(
    sf
    docopt
    serialport
    serialmac
    observer
    stringhex
    )

target_include_directories(sf PUBLIC ${PROJECT_SOURCE_DIR})

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

if (WIN32)
    MESSAGE(STATUS "On Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
    # Not all compilers offer macros to indicate a Windows build environment.
    add_definitions(-D__WIN32__)
    find_library ( SETUPAPI setupapi )
    set(WIN32_DEPS ${SETUPAPI})
endif()


##
## EXTERNAL PROJECTS
##
# DOCOPT
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

if(WIN32)
  set(LIBSERIALPORT_CONFIGURE_PARAMETER --disable-shared)
else()
  set(LIBSERIALPORT_CONFIGURE_PARAMETER )
endif()

if(NOT LIBSERIALPORT)
    MESSAGE(STATUS "libserialport not found!")
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
        install_dir build_command configure_command)
    MESSAGE(STATUS "libserialport will be installed to:\n "
        ${install_dir} "\nusing build command: " ${build_command}
        "\nusing configure command: " ${configure_command})
    # re-run cmake in initial build
    # will update cmakecache/project files on first build
    # so you may have to reload project after first build
    add_custom_target(serialport COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} DEPENDS libserialport)
else()
    MESSAGE(STATUS "libserialport found!")
    # serialport becomes a dummy target after first build
    # this prevents cmake from rebuilding cache/projects on subsequent builds
    add_custom_target(serialport)
endif()

set(LIBSERIALPORT_HEADERS ${LIBSERIALPORT_INSTALL_DIR}/include/libserialport.h)
set_source_files_properties(${LIBSERIALPORT_HEADERS} PROPERTIES GENERATED TRUE)
include_directories(${LIBSERIALPORT_INSTALL_DIR}/include)


##
## INTERNAL PROJECTS
##
# SERIALMAC
add_subdirectory(${PROJECT_SOURCE_DIR}/../lib lib)
# OBSERVER
add_subdirectory(${PROJECT_SOURCE_DIR}/../dep/callback-observer observer)
# STRINGHEX
add_subdirectory(${PROJECT_SOURCE_DIR}/../dep/stringhex stringhex)


##
## LINKING
##
# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )

if(LIBSERIALPORT) # This is needed for integrating the non-CMake project
    target_link_libraries(sf
        serialmac
        ${LIBSERIALPORT}
        docopt
        observer
        stringhex
        ${CMAKE_THREAD_LIBS_INIT}
        ${WIN32_DEPS}
        )
endif()

##
## INSTALLATION
##
install(TARGETS sf DESTINATION bin)
