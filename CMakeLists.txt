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
## @brief      STACKFORCE Serial MAC Command Line Interface
##
## @details See @code sfserialcli --help @endcode for details.
##
## This file is part of the STACKFORCE Serial Command Line Interface
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
## along with serialmac cli. If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required(VERSION 3.6)
project(sfserialcli VERSION 1.0.3)

#---------------------------------------------------------------------------------------
# Include required modules
#---------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ColorMessages)

#---------------------------------------------------------------------------------------
# Required dependencies versions
#---------------------------------------------------------------------------------------
set(DOCOPT_VERSION 0.6.2)
set(SFOBSERVER_VERSION 3.0.0)
set(SFSERIALOBSERVER_VERSION 1.0.0)
set(SFSERIALMAC_VERSION 1.0.1)
set(SFSTRINGHEX_VERSION 0.2.0)
set(SFCRC_VERSION 0.1.3)

#---------------------------------------------------------------------------------------
# Product extra info
#---------------------------------------------------------------------------------------
set(PROJECT_PRODUCT_COMPANY     "STACKFORCE")
set(PROJECT_PRODUCT_NAME        "Serial MAC Command Line Interface")
set(SERIALMACCLI_PROGRAM_NAME   "sfserialcli")
set(SERIALMAC_VERSION           ${SFSERIALMAC_VERSION})

#---------------------------------------------------------------------------------------
# Find required dependencies
#---------------------------------------------------------------------------------------
colormessage(MORE_IMPORTANT "Searching required project dependencies...")

colormessage(STATUS "Searching docopt project configuration")
find_package(docopt ${DOCOPT_VERSION} REQUIRED)
colormessage(STATUS "Searching sfobserver project configuration")
find_package(sfobserver ${SFOBSERVER_VERSION} REQUIRED)
colormessage(STATUS "Searching sfserialobserver project configuration")
find_package(sfserialobserver ${SFSERIALOBSERVER_VERSION} REQUIRED)
colormessage(STATUS "Searching sfserialmac project configuration")
find_package(sfserialmac ${SFSERIALMAC_VERSION} REQUIRED)
colormessage(STATUS "Searching sfstringhex project configuration")
find_package(sfstringhex ${SFSTRINGHEX_VERSION} REQUIRED)
colormessage(STATUS "Searching sfcrc project configuration")
find_package(sfcrc ${SFCRC_VERSION} REQUIRED)
colormessage(STATUS "Searching Threads")
find_package(Threads REQUIRED)
colormessage(STATUS "Searching Serialport project configuration")
find_package(Serialport REQUIRED)

if(CMAKE_CROSSCOMPILING AND WIN32)
    colormessage(STATUS "Searching mingw threads for windows cross build")
    find_package(MingwThreads REQUIRED)
endif()

#---------------------------------------------------------------------------------------
# CMake options
#---------------------------------------------------------------------------------------
option(BUILD_STATIC "Build static executable" ON)

#---------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -fmessage-length=0")

if(WIN32)
    if(CMAKE_CROSSCOMPILING)
        message(STATUS "Cross compiling for Windows")
        message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
        add_definitions(-D__WIN32_CROSS_BUILD__)
    else()
        message(STATUS "Windows native")
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

#---------------------------------------------------------------------------------------
# Get include directories
#---------------------------------------------------------------------------------------
if(CMAKE_CROSSCOMPILING AND WIN32)
	get_target_property(MINGW_THREADS_INCLUDE_DIRS mingw-std-threads INTERFACE_INCLUDE_DIRECTORIES)
endif()

#---------------------------------------------------------------------------------------
# Add subdirectories
#---------------------------------------------------------------------------------------
add_subdirectory(src)
