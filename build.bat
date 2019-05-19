@echo off

pushd ..\build\debug

cl -Wall -wd4201 -wd4505 -wd4514 -Z7 -FC ..\..\code\ast_main.cpp user32.lib d3d11.lib D3DCompiler.lib

popd