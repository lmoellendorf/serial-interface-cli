# README
```
     ___ _____ _   ___ _  _____ ___  ___  ___ ___
    / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
    \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
    |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
    embedded.connectivity.solutions===============
```

# Introduction

Command Line Interface to easily send and receive serial messages from and to devices using the STACKFORCE [serial-interface-mac](https://github.com/stackforce/serial-interface-mac).

# Get Precompiled Binaries

## Windows

Precompiled binaries for Windows can be found in the [releases](https://github.com/stackforce/serial-interface-cli/releases) section. **Please note that currently Windows binaries have been tested under 64Bit Windows 7/10 only.**

Download and execute the installer.

## Ubuntu based Linux distributions

Precompiled binaries for Ubuntu based Linux distributions can be found in the [releases](https://github.com/stackforce/serial-interface-cli/releases) section. **Please note that currently DEB packages have been tested under Ubuntu-14.04 64Bit and Ubuntu-16.04 64Bit based systems only.**

Download the DEB package that suites your Ubuntu version and run the following command to install it, where **[path/to/deb]** is the path to the downloaded DEB package:

    $ sudo dpkg -i [path/to/deb]

# Usage

## Interactive

Following a simple example of how to send a ping command to a device attached on serial port **/dev/ttyACM0** using the default serial port settings and getting the according response

    $ sfserialcli -d /dev/ttyACM0
    a
    00 00 0A

Entering an empty line will cause the programm to quit.

**NOTE there is no need to specify SYNC, Length or CRC fields since this is handled by the underlying [serial-interface-mac](https://github.com/stackforce/serial-interface-mac) library.** Those fields get stripped out by the serial mac from the incoming responses as well, leaving the payload only.

Running sfserialcli with the **-h** parameter will show the program's usage help with further invocation options.

## Script

The sfserialcli tool can be used inside custom scripts by providing the payload to be sent as a parameter when the tool is invoked. The sfserialcli sends the provided payload and prints the received response to standard output so it can be stored in a variable for further use. A success/failure exit code is returned as well. Demo scripts can be found under the *"examples"* directory.

BASH demo script:

![cliBashDemo](doc/gif/cliBashDemo.gif)


# Build

## Build dependencies

* [crc](https://github.com/stackforce/crc)
* [stringhex](https://github.com/stackforce/stringhex)
* [observer](https://github.com/stackforce/observer)
* [serial mac](https://github.com/stackforce/serial-interface-mac)
* [serial observer](https://github.com/stackforce/serial-interface-observer)
* [docopt](https://github.com/docopt/docopt.cpp)

## GNU/Linux

The STACKFORCE serial-interface-cli uses CMake as build system.

**NOTE all required dependencies have to be available in order to build the CLI**. See section **Build dependencies** for a list of the required dependencies.

Go to the project directory and create a build subdirectory:

    cd serial-interface-cli
    git submodule update --init
    mkdir build
    cd build

and run:

    cmake ..
    make

## Cross building for Windows on GNU/Linux using MinGW toolchain

**NOTE: The dependencies list applies to Ubuntu 16.04 64Bit based distributions**

* binutils-mingw-w64-x86-64
* g++-mingw-w64
* g++-mingw-w64-x86-64
* gcc-mingw-w64
* gcc-mingw-w64-base
* gcc-mingw-w64-x86-64
* mingw-w64
* mingw-w64-common
* mingw-w64-tools
* mingw-w64-x86-64-dev

Cross building requires a toolchain file to be specified during the build process. You can use your own defined toolchain file. For simplicity a toolchain file named **toolchain-cross-mingw-x86_64.cmake** for MinGW is available in the project's **cmake/modules** subdirectory:

    cd serial-interface-cli
    git submodule update --init
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/modules/toolchain-cross-mingw-x86_64.cmake ..
    make

## Docker Build
Docker files can be used to generate different platforms packages. A convenience script to build a packages for a specific platform is provided under the _*docker*_ directory. Following example builds the _*ubuntu*_ DEB package using the helper script:

    cd docker
    ./buildpackage.sh <platform>

Where _*platform*_ is one of
* ubuntu (Ubuntu Xenial)
* debian (Debian Stretch)
* windows (Windows 10)

Generated packages are placed under the _*packages*_ subdirectory.