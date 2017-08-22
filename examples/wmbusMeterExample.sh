#!/bin/bash

# @code
#  ___ _____ _   ___ _  _____ ___  ___  ___ ___
# / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
# \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
# |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
# embedded.connectivity.solutions.==============
# @endcode
#
# @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
# @author     Adrian Antonana
# @brief      STACKFORCE Serial MAC Command Line Client script demo
#
# This file is part of the STACKFORCE Serial Command Line Client
# (below "serialmac cli").
#
# The serialmac cli is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The serialmac cli is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with libserialmac.  If not, see <http://www.gnu.org/licenses/>.

# Usage
# -----
# Set the CLI_EXEC variable with the full path to the sfserialcli executable if
# the sfserialcli executable is not in your path, set the CLI_EXEC to "sfserialcli"
# otherwise.
# The default serial port SERIAL_PORT can be overriden by pasing a port as script
# parameter.
#
# Invocation example
# ------------------
# * Under Linux with serial port ttyACM1:
#                                 ./wmbusMeterExample.sh /dev/ttyACM1
#
# * Under MinGW with COM port 16:
#                                 ./wmbusMeterExample.sh com16

#---------------------------------------------------------------------------------------
# general settings
#---------------------------------------------------------------------------------------
SERIAL_PORT="/dev/ttyACM0"
SUCCESS_COUNT=0
FAILURE_COUNT=0

#---------------------------------------------------------------------------------------
# color codes
#---------------------------------------------------------------------------------------
CYAN="\e[0;36m"
GREEN="\e[0;32m"
RED="\e[0;31m"
YELLOW="\e[0;33m"
NO_COLOR="\e[0m"

#---------------------------------------------------------------------------------------
# wmbus commands
#---------------------------------------------------------------------------------------
PING_CMD="0A"
PING_CMD_RESP="00 00 0A"
SET_ADDRESS_CMD="05 15"
SET_ADDRESS_CMD_RESP="00 00 05 15"
GET_ADDRESS_CMD="06 15"
GET_ADDRESS_CMD_RESP="02 06 15"

#---------------------------------------------------------------------------------------
# meter addresses
#---------------------------------------------------------------------------------------
ORIG_METER_ADDRESS="D1 33 80 00 00 01 23 07"
NEW_METER_ADDRESS="D1 33 80 00 00 01 23 08"

#---------------------------------------------------------------------------------------
# function definitions
#---------------------------------------------------------------------------------------
function sendExpect {
    [[ "$("$CLI_EXEC" -d $SERIAL_PORT $1)" == "$2" ]] && { ((SUCCESS_COUNT++)); SEND_EXPECT_RET_VAL=0; } || { ((FAILURE_COUNT++)); SEND_EXPECT_RET_VAL=1; }
}

#---------------------------------------------------------------------------------------
# main part
#---------------------------------------------------------------------------------------
# detect OS env
echo -ne "${CYAN}OS${NO_COLOR}       : "
case "$(uname -s)" in
	Linux)
		echo "Linux"
		CLI_EXEC="sfserialcli"
		;;
	CYGWIN*|MINGW*|MSYS*)
		echo "Windows"
		CLI_EXEC="$PROGRAMFILES\\sfserialcli\\bin\\sfserialcli.exe"
		;;
	*)
		echo -e "${RED}unkown${NO_COLOR}"
		exit 1
		;;
esac

# check cli binary
echo -ne "${CYAN}CLI tool${NO_COLOR} : "
[[ -z "$(which $CLI_EXEC 2> /dev/null)" ]] && { echo -e "${RED}not found${NO_COLOR}"; exit 1; } || echo -e "${GREEN}${CLI_EXEC}${NO_COLOR}"

# paramter check
[[ ! -z "$1" ]] && SERIAL_PORT="$1"

echo -e "${CYAN}Serial port${NO_COLOR} : $SERIAL_PORT"
echo

# ping test
echo -e "${CYAN}>> Test ping command${NO_COLOR}"
echo -e "${YELLOW}send${NO_COLOR}   : $PING_CMD"
echo -e "${YELLOW}expect${NO_COLOR} : $PING_CMD_RESP"
sendExpect "$PING_CMD" "$PING_CMD_RESP"
echo -ne "${YELLOW}result${NO_COLOR} : "
[[ $SEND_EXPECT_RET_VAL == 0 ]] && echo -e "${GREEN}OK${NO_COLOR}" || echo -e "${RED}FAIL${NO_COLOR}"
echo

# get meter address test
echo -e "${CYAN}>> Test get meter address command${NO_COLOR}"
echo -e "${YELLOW}send${NO_COLOR}   : $GET_ADDRESS_CMD"
echo -e "${YELLOW}expect${NO_COLOR} : $GET_ADDRESS_CMD_RESP $ORIG_METER_ADDRESS"
sendExpect "$GET_ADDRESS_CMD" "$GET_ADDRESS_CMD_RESP $ORIG_METER_ADDRESS"
echo -ne "${YELLOW}result${NO_COLOR} : "
[[ $SEND_EXPECT_RET_VAL == 0 ]] && echo -e "${GREEN}OK${NO_COLOR}" || echo -e "${RED}FAIL${NO_COLOR}"
echo

# set meter address test
echo -e "${CYAN}>> Test set meter address command${NO_COLOR}"
echo -e "${YELLOW}send${NO_COLOR}   : $SET_ADDRESS_CMD $NEW_METER_ADDRESS"
echo -e "${YELLOW}expect${NO_COLOR} : $SET_ADDRESS_CMD_RESP"
sendExpect "$SET_ADDRESS_CMD $NEW_METER_ADDRESS" "$SET_ADDRESS_CMD_RESP"
echo -ne "${YELLOW}result${NO_COLOR} : "

if [[ $SEND_EXPECT_RET_VAL == 0 ]]; then
    echo -e "${GREEN}OK${NO_COLOR}"

    echo -e "${YELLOW}send${NO_COLOR}   : $GET_ADDRESS_CMD"
    echo -e "${YELLOW}expect${NO_COLOR} : $GET_ADDRESS_CMD_RESP $NEW_METER_ADDRESS"
    sendExpect "$GET_ADDRESS_CMD" "$GET_ADDRESS_CMD_RESP $NEW_METER_ADDRESS"
    echo -ne "${YELLOW}result${NO_COLOR} : "

    if [[ $SEND_EXPECT_RET_VAL == 0 ]]; then
        echo -e "${GREEN}OK${NO_COLOR}"
    else
        echo -e "${RED}FAIL${NO_COLOR}"
    fi
else
    echo -e "${RED}FAIL${NO_COLOR}"
fi
echo

# set back original meter address
echo -e "${CYAN}>> Set back original meter address${NO_COLOR}"
echo -e "${YELLOW}send${NO_COLOR}   : $SET_ADDRESS_CMD $ORIG_METER_ADDRESS"
echo -e "${YELLOW}expect${NO_COLOR} : $SET_ADDRESS_CMD_RESP"
sendExpect "$SET_ADDRESS_CMD $ORIG_METER_ADDRESS" "$SET_ADDRESS_CMD_RESP"
echo -ne "${YELLOW}result${NO_COLOR} : "

if [[ $SEND_EXPECT_RET_VAL == 0 ]]; then
    echo -e "${GREEN}OK${NO_COLOR}"

    echo -e "${YELLOW}send${NO_COLOR}   : $GET_ADDRESS_CMD"
    echo -e "${YELLOW}expect${NO_COLOR} : $GET_ADDRESS_CMD_RESP $ORIG_METER_ADDRESS"
    sendExpect "$GET_ADDRESS_CMD" "$GET_ADDRESS_CMD_RESP $ORIG_METER_ADDRESS"
    echo -ne "${YELLOW}result${NO_COLOR} : "

    if [[ $SEND_EXPECT_RET_VAL == 0 ]]; then
        echo -e "${GREEN}OK${NO_COLOR}"
    else
        echo -e "${RED}FAIL${NO_COLOR}"
    fi
else
    echo -e "${RED}FAIL${NO_COLOR}"
fi

echo
echo -e "${CYAN}>> Test summary${NO_COLOR}"
echo -e "${YELLOW}Successful commands${NO_COLOR} : $SUCCESS_COUNT"
echo -e "${YELLOW}Failed commands${NO_COLOR}     : $FAILURE_COUNT"
echo
[[ $FAILURE_COUNT == 0 ]] && echo -e "${GREEN}All commands run successfully${NO_COLOR}"
