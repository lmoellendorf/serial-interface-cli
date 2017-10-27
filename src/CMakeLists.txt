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

#---------------------------------------------------------------------------------------
# cli headers and sources
#---------------------------------------------------------------------------------------
set(SF_SERIALMACCLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sf_serialmaccli.cpp
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------
add_executable(sfserialcli ${SF_SERIALMACCLI_SOURCES})

target_include_directories(sfserialcli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
    target_include_directories(sfserialcli PRIVATE
        ${MINGW_THREADS_INCLUDE_DIRS}
    )
endif()

if(BUILD_STATIC)
    set(LINK_LIBS sfserialmac-static docopt_s sfobserver-static sfserialobserver-static sfstringhex-static)
else()
    set(LINK_LIBS sfserialmac docopt sfserialobserver sfstringhex)
endif()

target_link_libraries(sfserialcli PRIVATE
    ${LINK_LIBS}
    libserialport-static
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
        COMPONENT runtime
)

# install windows specific files
if(WIN32)
    install(FILES ${CMAKE_SOURCE_DIR}/cmake/cpack/startcli.cmd ${CMAKE_SOURCE_DIR}/cmake/cpack/sf.ico
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT runtime
    )
endif()

# install version file
install(FILES ${PROJECT_VERSION_FILE}
        DESTINATION ${CONFIG_INSTALL_DIR}
        COMPONENT dev
)

# build directory package config
export(EXPORT ${TARGETS_EXPORT_NAME}
       FILE ${PROJECT_CONFIG_FILE}
)

# installed package config
install(EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION ${CONFIG_INSTALL_DIR}
        FILE ${PROJECT_CONFIG_FILE}
        COMPONENT dev
)

# add to CMake registry
export(PACKAGE ${PROJECT_NAME})

#---------------------------------------------------------------------------------------
# Create release packages
#---------------------------------------------------------------------------------------
# package release version
set(PACKAGE_RELEASE_VERSION 1)

# general CPack config
set(CPACK_COMPONENTS_ALL runtime) # generate packages for runtime only
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME sfserialcli)
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Serial Interface Command Line Tool")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
message(STATUS "CPack: packages will be generated under ${CPACK_PACKAGE_DIRECTORY}")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_PRODUCT_COMPANY} ${PROJECT_PRODUCT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_PRODUCT_COMPANY} GmbH")
set(CPACK_PACKAGE_CONTACT "info@stackforce.de")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

set(CPACK_STRIP_FILES ON)

# determine distribution and architecture
if(NOT WIN32)
    execute_process(COMMAND lsb_release -si OUTPUT_VARIABLE LINUX_DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -sr OUTPUT_VARIABLE LINUX_DISTRO_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE MACHINE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(TOLOWER ${LINUX_DISTRO} LINUX_DISTRO)

    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${PACKAGE_RELEASE_VERSION}_Linux-${MACHINE_ARCH})

    if(LINUX_DISTRO STREQUAL "debian" OR LINUX_DISTRO STREQUAL "ubuntu" OR LINUX_DISTRO STREQUAL "linuxmint")
        execute_process(COMMAND dpkg-architecture -qDEB_HOST_ARCH OUTPUT_VARIABLE DEBIAN_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND CPACK_GENERATOR DEB)
        set(CPACK_DEB_COMPONENT_INSTALL ON)
        set(CPACK_DEBIAN_RUNTIME_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${PACKAGE_RELEASE_VERSION}_${LINUX_DISTRO}-${LINUX_DISTRO_VERSION}_${DEBIAN_ARCH}.deb)
        set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
        set(CPACK_DEBIAN_PACKAGE_RELEASE ${PACKAGE_RELEASE_VERSION})
        set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

        if(NOT BUILD_STATIC)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsfcrc (= ${SFCRC_VERSION}), libsfobserver (= ${SFOBSERVER_VERSION}), libsfserialmac (= ${SFSERIALMAC_VERSION}), libsfstringhex (= ${SFSTRINGHEX_VERSION}), docopt (= ${DOCOPT_VERSION})")
        endif()

        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/stackforce")
        set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    endif()
else()
    list(APPEND CPACK_GENERATOR NSIS)
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${PACKAGE_RELEASE_VERSION}_win64)
    set(CPACK_NSIS_PACKAGE_NAME "Serial Command Line Tool")
    set(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/cmake/cpack/sf.ico)
    set(CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/cmake/cpack/sf.ico)
    set(CPACK_NSIS_INSTALLED_ICON_NAME ${CMAKE_SOURCE_DIR}/cmake/cpack/sf.ico)
    set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/cmake/cpack/sf_logo_heading.bmp)
    set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP ${CMAKE_SOURCE_DIR}/cmake/cpack/sf_logo_margin.bmp)
    set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP ${CMAKE_SOURCE_DIR}/cmake/cpack/sf_logo_margin.bmp)
    set(CPACK_NSIS_HELP_LINK "www.stackforce.de")
    set(CPACK_NSIS_URL_INFO_ABOUT "www.stackforce.de")
    set(CPACK_NSIS_CONTACT "info@stackforce.de")

    set(CPACK_PACKAGE_EXECUTABLES "sfserialcli" "sfserialcli")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "sfserialcli")
    set(CPACK_NSIS_MENU_LINKS
        "https://github.com/stackforce/serial-interface-cli" "Serial CLI project on GitHub"
        "https://www.stackforce.de" "STACKFORCE Home Page"
    )

#     set(CPACK_CREATE_DESKTOP_LINKS "sfserialcli")
    set(CPACK_NSIS_INSTALLED_ICON_NAME $INSTDIR\\\\bin\\\\sf.ico)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\sfserialcli.lnk' '$INSTDIR\\\\bin\\\\startcli.cmd' '' '$INSTDIR\\\\bin\\\\sf.ico'
        CreateShortCut '$DESKTOP\\\\sfserialcli.lnk' '$INSTDIR\\\\bin\\\\startcli.cmd' '' '$INSTDIR\\\\bin\\\\sf.ico'"
    )

    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\sfserialcli.lnk'
        Delete '$DESKTOP\\\\sfserialcli.lnk'"
    )

#     set(CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)
