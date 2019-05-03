@echo off

pushd ..\build\debug

echo remove D3DX11.lib

cl -Wall -Zi -FC ..\..\code\ast_main.cpp user32.lib d3d11.lib D3DCompiler.lib D3DX11.lib


popd