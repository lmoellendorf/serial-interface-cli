cmake_minimum_required (VERSION 2.8)
project (serialcli)

include(ExternalProject)
include(CTest)

add_subdirectory(${PROJECT_SOURCE_DIR}/../lib ${PROJECT_BINARY_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/../dep/observer
    ${PROJECT_BINARY_DIR}/observer)
add_subdirectory(${PROJECT_SOURCE_DIR}/../tst/gunit ${PROJECT_BINARY_DIR}/tst)

set (SERIALMACCLI_PRODUCT_NAME "sf-serial" CACHE STRING "product")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
    )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic -fmessage-length=0")

if (WIN32)
    MESSAGE(STATUS "On Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
    find_library ( SETUPAPI setupapi )
    set(WIN32_DEPS ${SETUPAPI})
endif()

##
## DOCOPT
##
ExternalProject_Add(libdocopt   # Name for custom target
    #--Download step--------------
    GIT_REPOSITORY https://github.com/docopt/docopt.cpp.git
    GIT_TAG master
    #--Configure step-----------------
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )

externalproject_get_property(libdocopt install_dir)
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

ExternalProject_Get_Property(libdocopt install_dir binary_dir)
include_directories("${install_dir}/include/docopt")


set(LIBSERIALPORT_INSTALL_DIR ${PROJECT_BINARY_DIR}/libserialport-prefix)
find_library(LIBSERIALPORT serialport
    ${LIBSERIALPORT_INSTALL_DIR}/lib/)

if(NOT LIBSERIALPORT)
    MESSAGE(STATUS "libserialport not found!")
    ExternalProject_Add(libserialport    # Name for custom target
        #--Download step--------------
        GIT_REPOSITORY git://sigrok.org/libserialport
        GIT_TAG master
        #--Configure step-------------
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
    add_custom_target(serialport)
endif()

include_directories(
    # add the binary tree to the search path for include files
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../lib
    ${LIBSERIALPORT_INSTALL_DIR}/include
    ${PROJECT_SOURCE_DIR}/../dep/observer
    )

# This is only added to display the header in your IDE!
set(LIBSERIALPORT_HEADERS
    ${LIBSERIALPORT_INSTALL_DIR}/include/libserialport.h
    )
set_source_files_properties(${LIBSERIALPORT_HEADERS} PROPERTIES GENERATED TRUE)

# This is only added to display the files in your IDE!
set(OBERSERVER_FILES
    ${PROJECT_SOURCE_DIR}/../dep/observer/sf_event.h
    ${PROJECT_SOURCE_DIR}/../dep/observer/sf_event.cpp
    ${PROJECT_SOURCE_DIR}/../dep/observer/sf_observer.h
    ${PROJECT_SOURCE_DIR}/../dep/observer/sf_subject.h
    ${PROJECT_SOURCE_DIR}/../dep/observer/sf_subject.cpp
    )

set(SF_SERIALMACCLI_HEADERS
    ${LIBSERIALPORT_HEADERS}
    ${PROJECT_BINARY_DIR}/version.h
    ${PROJECT_SOURCE_DIR}/sf_serialmaccli.h
    ${PROJECT_SOURCE_DIR}/sf_serialmachandler.h
    ${PROJECT_SOURCE_DIR}/sf_stringhex.h
    #${PROJECT_SOURCE_DIR}/../tst/gunit/tests/sf_teststringhex.h
    )

set(SF_SERIALMACCLI_SOURCES
    ${PROJECT_SOURCE_DIR}/main
    ${PROJECT_SOURCE_DIR}/sf_serialmaccli.cpp
    ${PROJECT_SOURCE_DIR}/sf_serialmachandler.cpp
    ${PROJECT_SOURCE_DIR}/sf_stringhex.cpp
    #${PROJECT_SOURCE_DIR}/../tst/gunit/tests/sf_teststringhex.cpp
    ${SF_SERIALMACCLI_HEADERS}
    ${OBERSERVER_FILES}
    )

add_executable(sf  ${SF_SERIALMACCLI_SOURCES})

add_dependencies(sf
    docopt
    serialport
    serialmac
    observer
    crc
    )

# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )

if(LIBSERIALPORT)
    target_link_libraries(sf
        serialmac
        ${LIBSERIALPORT}
        docopt
        observer
        ${CMAKE_THREAD_LIBS_INIT}
        ${WIN32_DEPS}
        crc
        )
endif()

add_test(sf-stringhex-test tst/sf-stringhex-test)
