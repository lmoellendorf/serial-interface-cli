cmake_minimum_required (VERSION 2.6)
project (serial-interface-gunit)

include(ExternalProject)
include(CTest)

set (COMMON_COMPILER_FLAGS "")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILER_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${COMMON_COMPILER_FLAGS}")

#if (WIN32)
#    MESSAGE(STATUS "On Windows")
#    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#endif()

set(GOOGLETEST_INSTALL_DIR ${PROJECT_BINARY_DIR}/googletest-prefix/src/)
message(STATUS "GOOGLETEST_INSTALL_DIR ${GOOGLETEST_INSTALL_DIR}")
set(GOOGLETEST_SOURCDE_DIR ${GOOGLETEST_INSTALL_DIR}/googletest/)
message(STATUS "GOOGLETEST_SOURCDE_DIR ${GOOGLETEST_SOURCDE_DIR}")
set(GOOGLETEST_BUILD_DIR ${GOOGLETEST_INSTALL_DIR}/googletest-build)
message(STATUS "GOOGLETEST_BUILD_DIR ${GOOGLETEST_BUILD_DIR}")
set(GOOGLETEST_GMOCK_SOURCE_DIR ${GOOGLETEST_SOURCDE_DIR}/googlemock/)
message(STATUS "GOOGLETEST_GMOCK_SOURCE_DIR ${GOOGLETEST_GMOCK_SOURCE_DIR}")
set(GOOGLETEST_GTEST_SOURCE_DIR ${GOOGLETEST_SOURCDE_DIR}/googletest/)
message(STATUS "GOOGLETEST_GTEST_SOURCE_DIR ${GOOGLETEST_GTEST_SOURCE_DIR}")
set(GOOGLETEST_GMOCK_INCLUDE_DIR ${GOOGLETEST_GMOCK_SOURCE_DIR}/include/)
message(STATUS "GOOGLETEST_GMOCK_INCLUDE_DIR ${GOOGLETEST_GMOCK_INCLUDE_DIR}")
set(GOOGLETEST_GTEST_INCLUDE_DIR ${GOOGLETEST_GTEST_SOURCE_DIR}/include/)
message(STATUS "GOOGLETEST_GTEST_INCLUDE_DIR ${GOOGLETEST_GTEST_INCLUDE_DIR}")
find_library(GOOGLEMOCK gmock ${GOOGLETEST_BUILD_DIR}/)

if(NOT GOOGLEMOCK)
  MESSAGE(STATUS "googlemock not found!")
  ExternalProject_Add(googletest   # Name for custom target
      #--Download step--------------
      GIT_REPOSITORY https://github.com/google/googletest.git
                     #git@github.com:docopt/docopt.cpp.git
      GIT_TAG master
      #--Configure step-----------------
      #--Since the CMakeLists.txt is in a subdirectory of SOURCE_DIR
      #--it has to be passed to cmake.
      #--Please note, that SOURCE_DIR determines the root of the GIT_REPOSITORY
      #--therefore changing it directly does not work.
      CONFIGURE_COMMAND cmake ${GOOGLETEST_GMOCK_SOURCE_DIR}
      #--Install step-----------------
      #--Citing from autotools:
      #-->'make install' is dangerous and not supported. Instead, see README
      #-->for how to integrate Google Test into your build system.
      INSTALL_COMMAND ""
      )
  ExternalProject_Get_Property(googletest SOURCE_DIR)
  MESSAGE(STATUS "googletest will be build in:\n " ${SOURCE_DIR})
  # rerun cmake in initial build
  # will update cmakecache/project files on first build
  # so you may have to reload project after first build
  add_custom_target(gmock ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS googletest)
else()
  MESSAGE(STATUS "googlemock found!")
  # serialport becomes a dummy target after first build
  # this prevents cmake from rebuilding cache/projects on subsequent builds
  add_custom_target(gmock)
  #add_subdirectory(${GOOGLETEST_GMOCK_SOURCE_DIR}/ ${PROJECT_BINARY_DIR}/gmock)
endif()

set(GMOCK_HEADERS
    ${GOOGLETEST_GMOCK_INCLUDE_DIR}/gmock/gmock.h
    ${GOOGLETEST_GTEST_INCLUDE_DIR}/gtest/gtest.h
)
set_source_files_properties(${GMOCK_HEADERS} PROPERTIES GENERATED TRUE)


# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/src_under_test/")
include_directories("${PROJECT_SOURCE_DIR}/mocks/")
include_directories("${PROJECT_SOURCE_DIR}/tests/")
include_directories("${GOOGLETEST_GMOCK_INCLUDE_DIR}")
include_directories("${GOOGLETEST_GTEST_INCLUDE_DIR}")


add_definitions (-D_cplusplus -DGTEST_HAS_PTHREAD)

set (SOURCES
    ${PROJECT_SOURCE_DIR}/tests/test
    ${PROJECT_SOURCE_DIR}/tests/test_sample
    ${PROJECT_SOURCE_DIR}/tests/printHex
    ${PROJECT_SOURCE_DIR}/src_under_test/foobar
    ${PROJECT_SOURCE_DIR}/mocks/mocked_foobar_extern
    ${GMOCK_HEADERS}
    )


add_executable(sf-gunit ${SOURCES})
add_dependencies(sf-gunit
    gmock
    )

# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )

if(GOOGLEMOCK)
    MESSAGE(STATUS "libgmock is:\n "
        ${GOOGLEMOCK})
  target_link_libraries (sf-gunit
      ${GOOGLEMOCK}
      ${CMAKE_THREAD_LIBS_INIT})
endif()
