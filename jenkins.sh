#!/bin/bash --

BUILD_DIR="build.jenkins"
# brutally clean build directory
rm -rf ${BUILD_DIR} >/dev/null 2>&1
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

if [[ $OSTYPE == "msys" ]]
then
    echo "Detected MSYS"
    echo "See
https://redmine.stackforce.de/projects/wheelstore/wiki/GCC#Native-build-using-MSYS2-and-MinGW-w64
on how to install the toolchain
and
https://redmine.stackforce.de/projects/wheelstore/wiki/CMake#Windows
on how to install CMake
properly!"
    cmake -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Debug  ../
else
    # default generator is fine
    cmake -D CMAKE_BUILD_TYPE=Debug  ../
fi

# build everything
make all

# run tests
#cli/test*

