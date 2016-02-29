##
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
## @brief      STACKFORCE Serial MAC Module
##
## This file is part of the STACKFORCE Serial Interface project
## (below "sf serial").
##
## sf serial is free software: you can redistribute it and/or modify
## it under the terms of the GNU Affero General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## sf serial is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Affero General Public License for more details.
##
## You should have received a copy of the GNU Affero General Public License
## along with sf serial.  If not, see <http://www.gnu.org/licenses/>.
##
cmake_minimum_required (VERSION 3.2)

find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Configure the project for testing
include(CTest)

if(WIN32)
  # Properly disable phtreads when compiling with MinGW
  # fix #606
  set(GTEST_GIT_REPOSITORY https://github.com/andoks/googletest.git)
  # This WIN32 switch can be removed as soon as this commit is in mainline
  set(GTEST_GIT_TAG 1f40320c1cb94d1c184d4e6308c1fe3cdca827ea)
  set(GTEST_PATCH_COMMAND echo "add_subdirectory(googlemock)" > CMakeLists.txt)
else()
  set(GTEST_GIT_REPOSITORY https://github.com/google/googletest.git)
  set(GTEST_GIT_TAG ddb8012eb48bc203aa93dcc2b22c1db516302b29)
  set(GTEST_PATCH_COMMAND)
endif()

ExternalProject_Add(googletest   # Name for custom target

  GIT_REPOSITORY ${GTEST_GIT_REPOSITORY}

  GIT_TAG ${GTEST_GIT_TAG}

  # Never update automatically from the remote repository
  UPDATE_DISCONNECTED 1

  PATCH_COMMAND ${GTEST_PATCH_COMMAND}

  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googletest source_dir)
MESSAGE(STATUS "google source_dir is:\n " ${source_dir} )


add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock googletest)

# Set gmock properties
ExternalProject_Get_Property(googletest source_dir binary_dir)
set_target_properties(libgmock PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
)


include_directories("${source_dir}/googletest/include/")
include_directories("${source_dir}/googlemock/include/")
