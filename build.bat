@echo off

pushd ..\build\debug

cl -Wall -Zi -FC ..\..\code\ast_main.cpp user32.lib

popd