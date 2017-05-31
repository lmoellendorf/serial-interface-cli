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
## along with serialmac cli. If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required(VERSION 3.6)
project(sfserialcli VERSION 0.1.2)

#---------------------------------------------------------------------------------------
# Include required modules
#---------------------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ExternalProject)

#---------------------------------------------------------------------------------------
# Product extra info
#---------------------------------------------------------------------------------------
set(PROJECT_PRODUCT_COMPANY "STACKFORCE"						CACHE STRING "company")
set(PROJECT_PRODUCT_NAME    "Serial MAC Command Line Client"	CACHE STRING "product")

#---------------------------------------------------------------------------------------
# Required dependencies versions
#---------------------------------------------------------------------------------------
set(DOCOPT_VERSION 0.6.2)
set(SFOBSERVER_VERSION 2.0.0)
set(SFSERIALMAC_VERSION 0.1.2)
set(SFSTRINGHEX_VERSION 0.2.0)
set(SFCRC_VERSION 0.1.2)

#---------------------------------------------------------------------------------------
# Find required dependencies
#---------------------------------------------------------------------------------------
find_package(docopt ${DOCOPT_VERSION} REQUIRED)
find_package(sfobserver ${SFOBSERVER_VERSION} REQUIRED)
find_package(sfserialmac ${SFSERIALMAC_VERSION} REQUIRED)
find_package(stringhex ${SFSTRINGHEX_VERSION} REQUIRED)
find_package(sfcrc ${SFCRC_VERSION} REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "Searching serialport project configuration")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
find_package(Serialport REQUIRED)
get_target_property(SERIALPORT_LIB libserialport IMPORTED_LOCATION)
get_target_property(SERIALPORT_LIB_STATIC libserialport-static IMPORTED_LOCATION)
message(STATUS "serial port lib: ${SERIALPORT_LIB}")
message(STATUS "serial port static lib: ${SERIALPORT_LIB_STATIC}")

#---------------------------------------------------------------------------------------
# CMake options
#---------------------------------------------------------------------------------------
option(BUILD_STATIC "Build static executable" ON)

#---------------------------------------------------------------------------------------
# Add subdirectories
#---------------------------------------------------------------------------------------
add_subdirectory(src)
