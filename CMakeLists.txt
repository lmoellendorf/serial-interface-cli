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
project(sfserialcli VERSION 0.1.2)

#---------------------------------------------------------------------------------------
# CMake options
#---------------------------------------------------------------------------------------
option(BUILD_STATIC "Build static executable" ON)

add_subdirectory(src)
