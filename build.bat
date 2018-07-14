@echo off
title Field Manager build script

if not exist debug (
    mkdir debug
)

if not exist release (
    mkdir release
)

cmd /c mingw32-make.exe all
pause
