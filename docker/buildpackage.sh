#!/usr/bin/env bash

#  ___ _____ _   ___ _  _____ ___  ___  ___ ___
# / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
# \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
# |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
# embedded.connectivity.solutions.==============
#
# copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
# author     Adrian Antonana <adrian.antonana@stackforce.de>
# brief      STACKFORCE Serial MAC CLI Docker build script



 TARGET_OS="$1"
 PROJECT_NAME="serial-interface-cli"

 case $TARGET_OS in
    "ubuntu" | "debian" | "windows")
                echo "Building ${PROJECT_NAME}"
                ;;
            *)
                echo "ERROR: Unsupported target OS \"${TARGET_OS}\""
                echo "supported targets are:"
                echo "   ubuntu"
                echo "   debian"
                echo "   windows"
                exit 1
                ;;
 esac

 IMAGE_TAG="${TARGET_OS}:${PROJECT_NAME}"
 IMAGE_PACKAGES_PATH="/work/${PROJECT_NAME}/build/packages/"
 docker build -f Dockerfile.${TARGET_OS} -t ${IMAGE_TAG} .. && \
 CONTAINER_ID=$(docker create ${IMAGE_TAG}) && \
 docker cp ${CONTAINER_ID}:${IMAGE_PACKAGES_PATH} . && \
 docker rm ${CONTAINER_ID}
