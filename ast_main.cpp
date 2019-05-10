/*
---------------- TODO LIST ----------------
- Fullscreen mode + switching with atl+enter
- Offline shader compilation
- HLSL error checking
- Allow different resolutions
*/

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)
#include "common_types.h"
#include "win32_d3d.cpp"

internal void Update(void)
{
	
}

internal LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (msg) {
		case WM_CLOSE: {
			PostQuitMessage(0);
		} break;
		default: {
			result = DefWindowProc(hwnd, msg, wParam, lParam);
		} break;
	}
	return(result);
}

#pragma warning(push, 0)
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
#pragma warning(pop)
{
	win32_d3d_program program;
	if (Win32D3DInitEverything(&program, MainWindowProc)) {
		b32 running = true;
		while (running) {
			MSG msg;
			while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
				if (msg.message==WM_QUIT) running = false;
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}
			float clearColour[4] = {0.0f, 0.2f, 0.4f, 1.0f};
			program.context.deviceContext->ClearRenderTargetView(program.context.renderTargetView, clearColour);
			
			Update();
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			
			program.context.deviceContext->Draw(4, 0);
			
			program.context.swapChain->Present(1, 0);
		}
	} else {
		Win32ShowErrorBox("Failed to init Win32 and D3D");
	}
	return(0);
}