@echo off
rem @code
rem  ___ _____ _   ___ _  _____ ___  ___  ___ ___
rem / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
rem \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
rem |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
rem embedded.connectivity.solutions.==============
rem @endcode
rem
rem @copyright  STACKFORCE GmbH, Heitersheim, Germany, www.stackforce.de
rem @author     David Rahusen
rem @brief      STACKFORCE Serial MAC Command Line Client script demo
rem
rem This file is part of the STACKFORCE Serial Command Line Client
rem (below "serialmac cli").
rem
rem The serialmac cli is free software: you can redistribute it and/o
rem it under the terms of the GNU Affero General Public License as pu
rem the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem
rem The serialmac cli is distributed in the hope that it will be usef
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU Affero General Public License for more details.
rem
rem You should have received a copy of the GNU Affero General Public 
rem along with libserialmac.  If not, see <http://www.gnu.org/license


rem === Init ===
cls
set output=
set input=
set sfcmd=
set error=

rem === Welcome ===
echo.
echo.
echo  ___ _____ _   ___ _  _____ ___  ___  ___ ___
echo / __^|_   _/_\ / __^| ^|/ / __/ _ \^| _ \/ __^| __^|
echo \__ \ ^| ^|/ _ \ (__^| ' ^<^| _^| (_) ^|   / (__^| _^|
echo ^|___/ ^|_/_/ \_\___^|_^|\_\_^| \___/^|_^|_\\___^|___^|
echo embedded.connectivity.solutions.==============
echo.
echo.
echo Welcome to STACKFORCE Demo script explaining how to use and control devices that are providing a STACKFORCE serial interface.
echo.
pause

rem === Select port and do basic explanation ===
cls
echo.
echo Please specify the COM port to be used, e.g. by simply typing COM6.
if not "%sfcomport%"=="" echo When leaving input blank and simply hit 'Enter', the last port '%sfcomport%' will be used.
set /p sfcomport=COM Port? 
set sfcmd=sfserialcli.exe -d %sfcomport%
timeout /t 1 > NUL
echo.
echo Thanks, from now on we will use the command line tool for any communication with the device just this:
echo.
echo '%sfcmd% xx yy zz ...'
echo.
echo Whereas 'xx yy zz' represents the wanted data bytes to be sent to the device.
echo.
echo Please keep in mind:
echo The command-line tool 'sfserialcli' performs the full STACKFORCE serial protocol in background. That means, when sending a simple '0A' using that tool (e.g. 'sfserialcli.exe -d COM3 0A'), in background the remaining protocol bytes will be added to the actual payload to be sent:
echo.
echo 1. SFD (Start Frame Delimiter) = 1 Byte
echo 2. Length = 2 Bytes
echo 3. Payload ... whatever the application has been passing to that tool
echo 4. CRC = 2 Bytes
echo.
pause

rem === Perform ping test ===
cls
echo.
echo First we send a PING command to the device in order to check if the device is present and accessible:
timeout /t 1 > NUL
call :SEND 0A
timeout /t 1 > NUL
echo.
echo Ok, we received the following response: %input%
timeout /t 1 > NUL
echo.
echo - First field  means, this is a confirmation on a received command.
echo - Second field is the actual confirmation code. In case of '00', the command has been executed correctly, everything Ok. For a complete list of possible confirmation codes, refer to the documentation and look for "Confirmation codes", e.g. SERIAL_CONFIRM_OK.
echo - Third field is a confirmation of the received command. E.g., when sending a ping '0A', the device should respond with this field set to '0A' as well.
echo.
pause

rem === Outro ===
cls
echo.
echo Thanks for using this script. Please feel free to extend it and create pull requests if you have any ideas for improving it. Constructive participation is higly appreciated. :-)
echo.
echo.
echo === THE FIN ===
echo.
echo.
pause
exit /B 0

:SEND
echo ^< %*
FOR /F "delims=" %%i IN ('%sfcmd% %*') DO set input=%%i
if "%input%" == "" (
    echo Something went wrong ... :-(
    exit /B 5
)
echo ^> %input%
exit /B 0
