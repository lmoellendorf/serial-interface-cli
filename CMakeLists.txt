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

cmake_minimum_required(VERSION 3.6)
project(sfserialcli)

#---------------------------------------------------------------------------------------
# Include required modules
#---------------------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ExternalProject)

#---------------------------------------------------------------------------------------
# Product and version
#---------------------------------------------------------------------------------------
set(${PROJECT_NAME}_VERSION_MAJOR   0)
set(${PROJECT_NAME}_VERSION_MINOR   1)
set(${PROJECT_NAME}_VERSION_PATCH   2)
set(${PROJECT_NAME}_PRODUCT_COMPANY "STACKFORCE" CACHE STRING "company")
set(${PROJECT_NAME}_PRODUCT_NAME    "Serial MAC Command Line Client" CACHE STRING "product")
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH} )

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
find_package(serialport CONFIG REQUIRED)
get_target_property(SERIALPORT_LIB libserialport IMPORTED_LOCATION)
get_target_property(SERIALPORT_LIB_STATIC libserialport-static IMPORTED_LOCATION)
message(STATUS "serial port lib: ${SERIALPORT_LIB}")
message(STATUS "serial port static lib: ${SERIALPORT_LIB_STATIC}")

#---------------------------------------------------------------------------------------
# cli headers and sources
#---------------------------------------------------------------------------------------
set(SF_SERIALMACCLI_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/sf_serialmaccli.cpp
    ${PROJECT_SOURCE_DIR}/src/sf_serialmachandler.cpp
)

configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
)

#---------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -fmessage-length=0")

if(WIN32)
	message(STATUS "On Windows")
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
	# Not all compilers offer macros to indicate a Windows build environment.
	add_definitions(-D__WIN32__)
	find_library(SETUPAPI setupapi)
	set(WIN32_DEPS ${SETUPAPI})
endif()

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------
add_executable(sfserialcli ${SF_SERIALMACCLI_SOURCES})

add_dependencies(
    sfserialcli
    serialport
)

target_include_directories(sfserialcli PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(sfserialcli
    sfserialmac-static
    libserialport-static
    sfcrc-static
    docopt_s
    sfobserver-static
    stringhex-static
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIN32_DEPS}
)

#---------------------------------------------------------------------------------------
# Install/export targets and files
#---------------------------------------------------------------------------------------
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(PROJECT_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE "${PROJECT_NAME}-config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

# generate package version file
write_basic_package_version_file(
    ${PROJECT_VERSION_FILE}
    VERSION ${${PROJECT_NAME}_VERSION}
    COMPATIBILITY SameMajorVersion
)

# install targets
install(TARGETS sfserialcli
        EXPORT ${TARGETS_EXPORT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# install version file
install(FILES ${PROJECT_VERSION_FILE}
        DESTINATION ${CONFIG_INSTALL_DIR}
)

# build directory package config
export(EXPORT ${TARGETS_EXPORT_NAME}
       FILE ${PROJECT_CONFIG_FILE}
)

# installed package config
install(EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION ${CONFIG_INSTALL_DIR}
        FILE ${PROJECT_CONFIG_FILE}
)

# add to CMake registry
export(PACKAGE ${PROJECT_NAME})

#---------------------------------------------------------------------------------------
# Create release packages
#---------------------------------------------------------------------------------------
# package release version
set(PACKAGE_RELEASE_VERSION 1)

# determine distribution and architecture
execute_process(COMMAND lsb_release -si OUTPUT_VARIABLE LINUX_DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND uname -m OUTPUT_VARIABLE MACHINE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)

# determine CPack generators
set(CPACK_GENERATOR "TGZ")

if(LINUX_DISTRO STREQUAL "Debian" OR LINUX_DISTRO STREQUAL "Ubuntu" OR LINUX_DISTRO STREQUAL "LinuxMint")
    set(CPACK_GENERATOR "DEB;TGZ")
endif()

# general CPack config
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
message(STATUS "CPack: packages will be generated under ${CPACK_PACKAGE_DIRECTORY}")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${${PROJECT_NAME}_PRODUCT_COMPANY} ${${PROJECT_NAME}_PRODUCT_NAME}")
set(CPACK_PACKAGE_VENDOR "${${PROJECT_NAME}_PRODUCT_COMPANY} GmbH")
set(CPACK_PACKAGE_CONTACT "info@stackforce.de")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_RELEASE_VERSION}-Linux-${MACHINE_ARCH})
set(CPACK_STRIP_FILES ON)

# debian specific CPack config
set(CPACK_DEBIAN_PACKAGE_RELEASE ${PACKAGE_RELEASE_VERSION})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsfcrc (>= ${SFCRC_VERSION})")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/stackforce")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

include(CPack)
