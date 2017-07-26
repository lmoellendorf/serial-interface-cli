@echo off
mode con:cols=120 lines=49
START "STACKFORCE Serial CLI" /B /LOW /WAIT "C:\Program Files\sfserialcli\bin\sfserialcli.exe" -h
cd "C:\Program Files\sfserialcli\bin"
cmd /k
