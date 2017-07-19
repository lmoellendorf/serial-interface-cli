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

Precompiled binaries for Windows can be found in the [releases](https://github.com/stackforce/serial-interface-cli/releases) section. **Please note that currently Windows binaries have been tested under Windows 10 only.**

Download and execute the installer. **When asked for it, select adding the program either to the system or user path.**

## Ubuntu based Linux distributions

Precompiled binaries for Ubuntu based Linux distributions can be found in the [releases](https://github.com/stackforce/serial-interface-cli/releases) section. **Please note that currently DEB packages have been tested under Ubuntu-14.04 and Ubuntu-16.04 based systems only.**

Download the DEB package that suites your Ubuntu version and run the following command to install it, where **[path/to/deb]** is the path to the downloaded DEB package:

    $ sudo dpkg -i [path/to/deb]

# Usage

Following a simple example of how to send a ping command to a device attached on serial port **/dev/ttyACM0** using the default serial port settings and getting the according response

    $ sfserialcli -d /dev/ttyACM0
    a
    00 00 0A

Entering an empty line will cause the programm to quit.

**NOTE there is no need to specify SYNC, Length or CRC fields since this is handled by the underlying [serial-interface-mac](https://github.com/stackforce/serial-interface-mac) library.** Those fields get stripped out by the serial mac from the incoming responses as well, leaving the payload only.

Running sfserialcli with the **-h** parameter will show the program's usage help with further invocation options.

# Build

## GNU/Linux

The STACKFORCE serial-interface-cli uses CMake as build system.

Go to the project directory and create a build subdirectory:

    cd serial-interface-cli
    git submodule update --init --recursive
    mkdir build
    cd build

and run:

    cmake ..
    make
    sudo make install

or to define a custom install directory e.g. devroot:

    cmake .. -DCMAKE_INSTALL_PREFIX=devroot
    make
    make install

## Cross building for Windows on GNU/Linux

Currently cross building for Windows is supported in the **win-cross-build** branch.
