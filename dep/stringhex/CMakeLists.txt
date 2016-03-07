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
## @brief      Cyclic Redundancy Check (CRC) error-detecting code.
##
cmake_minimum_required (VERSION 3.2)
project(stringhex)
# find our cmake modules
list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

add_library(stringhex ${PROJECT_SOURCE_DIR}/sf_stringhex)

# This is necessary for making the inclusion of this module via
# add_subdirectory() work.
target_include_directories(stringhex PUBLIC ${PROJECT_SOURCE_DIR})

###
### TESTING
###
find_package(gtest)

add_executable(teststringhex
    test/test.cpp
    test/sf_teststringhex.cpp
    sf_stringhex.cpp
    )
target_include_directories(teststringhex PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(teststringhex libgmock)
add_test(NAME teststringhex COMMAND teststringhex)
