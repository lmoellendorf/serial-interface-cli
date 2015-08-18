cmake_minimum_required (VERSION 2.8)
include(ExternalProject)

project (serialcli)

# The version number.
set (SF_SERIALMACCLI_VERSION_MAJOR 0)
set (SF_SERIALMACCLI_VERSION_MINOR 1)
set (SF_SERIALMACCLI_VERSION_PATCH 0)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic -fmessage-length=0")

if (WIN32)
    MESSAGE(STATUS "On Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
    )

add_subdirectory(${PROJECT_SOURCE_DIR}/../ ${PROJECT_BINARY_DIR}/lib)

# add the binary tree to the search path for include files
include_directories(
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/../src"
    "${PROJECT_BINARY_DIR}/libserialport-prefix/include/"
    )

find_library(LIBSERIALPORT serialport
    ${PROJECT_BINARY_DIR}/libserialport-prefix/lib/)

if(NOT LIBSERIALPORT)
    MESSAGE(STATUS "libserialport not found!")
    ExternalProject_Add(libserialport    # Name for custom target
        #PREFIX ${PROJECT_BINARY_DIR}
        #--Download step--------------
        GIT_REPOSITORY git://sigrok.org/libserialport
        GIT_TAG master
        #--Configure step-------------
        #SOURCE_DIR ${PROJECT_SOURCE_DIR}/../lib/libserialport/src
        # set prefix to the default install dir
        # (libserialport-prefix in this case)
        CONFIGURE_COMMAND ./autogen.sh && ./configure --disable-shared --prefix=<INSTALL_DIR>
        #--Build step-----------------
        BUILD_COMMAND make
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS 0
        #--Install step-----------------
        INSTALL_COMMAND make install
        )

    ExternalProject_Get_Property(libserialport install_dir)
    ExternalProject_Get_Property(libserialport build_command)
    ExternalProject_Get_Property(libserialport configure_command)
    MESSAGE(STATUS "libserialport will be installed to:\n "
        ${install_dir} "\nusing build command: " ${build_command}
        "\nusing configure command: " ${configure_command})
    # rerun cmake in initial build
    # will update cmakecache/project files on first build
    # so you may have to reload project after first build
    add_custom_target(serialport ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS libserialport)
else()
    MESSAGE(STATUS "libserialport found!")
    # serialport becomes a dummy target after first build
    # this prevents cmake from rebuilding cache/projects on subsequent builds
    ## TODO: Would be nice to have a target for cleaning/updating
    ## libserialport
    #add_custom_target(serialport ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS ${LIBSERIALPORT})
    add_custom_target(serialport)
endif()

set(LIBSERIALPORT_HEADER
    ${PROJECT_BINARY_DIR}/libserialport-prefix/include/libserialport.h
    )

set_source_files_properties(${LIBSERIALPORT_HEADER} PROPERTIES GENERATED TRUE)

set(SF_SERIALMACCLI_HEADERS
    ${LIBSERIALPORT_HEADER}
    ${PROJECT_BINARY_DIR}/version.h
    )

set(SF_SERIALMACCLI_SOURCES
    ${PROJECT_SOURCE_DIR}/main
    ${SF_SERIALMACCLI_HEADERS}
    )

add_executable(ssi  ${SF_SERIALMACCLI_SOURCES})

add_dependencies(ssi serialport serialmac)

# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )

if(LIBSERIALPORT)
    target_link_libraries(ssi
        serialmac
        ${LIBSERIALPORT}
        ${CMAKE_THREAD_LIBS_INIT}
        )
endif()

