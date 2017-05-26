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
## @author     Lars Moellendorf
## @author     Adrian Antonana
## @brief      STACKFORCE Serial MAC Command Line Client
##
## @details See @code sfserialcli --help @endcode for details.
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

cmake_minimum_required (VERSION 3.6)
project(sfserialcli)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ExternalProject)

#---------------------------------------------------------------------------------------
# Product and version
#---------------------------------------------------------------------------------------
set(${PROJECT_NAME}_VERSION_MAJOR   0)
set(${PROJECT_NAME}_VERSION_MINOR   1)
set(${PROJECT_NAME}_VERSION_PATCH   1)
set(${PROJECT_NAME}_PRODUCT_COMPANY "STACKFORCE" CACHE STRING "company")
set(${PROJECT_NAME}_PRODUCT_NAME    "Serial MAC Command Line Client" CACHE STRING "product")
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH} )

#---------------------------------------------------------------------------------------
# Required dependencies versions
#---------------------------------------------------------------------------------------
set(DOCOPT_VERSION 0.6.2)
# set(SFOBSERVER_VERSION 1.0.0)
set(SFSERIALMAC_VERSION 0.1.2)
set(SFSTRINGHEX_VERSION 0.2.0)
set(SFCRC_VERSION 0.1.2)

#---------------------------------------------------------------------------------------
# Find required dependencies
#---------------------------------------------------------------------------------------
find_package(docopt ${DOCOPT_VERSION} REQUIRED)
# find_package(observer ${OBSERVER_VERSION} REQUIRED)
find_package(sfserialmac ${SFSERIALMAC_VERSION} REQUIRED)
find_package(stringhex ${SFSTRINGHEX_VERSION} REQUIRED)
find_package(sfcrc ${SFCRC_VERSION} REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "Searching serialport project configuration")
find_package(serialport CONFIG REQUIRED)
get_target_property(SERIALPORT_LIB libserialport IMPORTED_LOCATION)
get_target_property(SERIALPORT_LIB_STATIC libserialport-static IMPORTED_LOCATION)
message(STATUS "serial port lib: ${SERIALPORT_LIB}")
message(STATUS "serial port static lib: ${SERIALPORT_LIB_STATIC}")

add_subdirectory(${PROJECT_SOURCE_DIR}/../dep/callback-observer observer)

#---------------------------------------------------------------------------------------
# cli headers and sources
#---------------------------------------------------------------------------------------
set(SF_SERIALMACCLI_SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/sf_serialmaccli.cpp
    ${PROJECT_SOURCE_DIR}/sf_serialmachandler.cpp
)

configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------
add_executable(sfserialcli  ${SF_SERIALMACCLI_SOURCES})

add_dependencies(
    sfserialcli
    serialport
    observer
)

target_include_directories(sfserialcli PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

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






target_link_libraries(sfserialcli
    sfserialmac-static
    libserialport-static
    sfcrc-static
    docopt_s
    observer
    stringhex-static
    ${CMAKE_THREAD_LIBS_INIT}
#     ${WIN32_DEPS}
)

##
## INSTALLATION
##
install(TARGETS sfserialcli DESTINATION bin)
