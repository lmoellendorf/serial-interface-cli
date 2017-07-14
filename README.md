# README
```
     ___ _____ _   ___ _  _____ ___  ___  ___ ___
    / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
    \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
    |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
    embedded.connectivity.solutions.==============
```

# Introduction

## Purpose

Command Line Interface to easily send and receive serial messages from and to devices using the STACKFORCE serial-interface-mac.

## Build instructions

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

## Cross Build for Windows instructions

** Cross build from Linux to Windows has been tested only under Ubuntu 16.04 based distributions with the MinGW toolchain installed.
Produced binaries should run under Windows 10. Other Windows versions have not been tested. **

To produce a Windows 64bit binary follow these steps after cloning the repository (use the i686 toolchain file for 32bit):

    cd serial-interface-cli
    git checkout win-cross-build
    git submodule update --init --recursive
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-cross-mingw-x86_64.cmake ..
    make

To generate an NSIS installer

    make package

## MinGW cross build dependencies

In order to be able to cross build in Linux to produce Windows 32bit and 64bit binaries following MinGW dependencies have to be met:

** NOTE: The dependencies list applies to Ubuntu 16.04 based distributions **

* binutils-mingw-w64-i686
* binutils-mingw-w64-x86-64
* g++-mingw-w64
* g++-mingw-w64-i686
* g++-mingw-w64-x86-64
* gcc-mingw-w64
* gcc-mingw-w64-base
* gcc-mingw-w64-i686
* gcc-mingw-w64-x86-64
* mingw-w64
* mingw-w64-common
* mingw-w64-i686-dev
* mingw-w64-tools
* mingw-w64-x86-64-dev
