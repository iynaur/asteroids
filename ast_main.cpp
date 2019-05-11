/*
---------------- TODO LIST ----------------
- Fullscreen mode + switching with atl+enter
- Offline shader compilation
- HLSL error checking
- Allow different resolutions
- Texture rendering
*/

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)
#include "common_types.h"
#include "win32_d3d.cpp"


struct game_state {
	vec2 playerPos;
	float playerRot;
};
internal game_state Update(void)
{
	game_state result;
	vec2 pos = {10.0f, 10.0f};
	persist float r = 0.0f;
	r += 0.1f;
	
	result.playerPos = pos;
	result.playerRot = r;
	return(result);
}

internal void UpdateConstBuffers(d3d_context context, ID3D11Buffer** constantBuffer, void* constantBufferData)
{
	context.deviceContext->UpdateSubresource(*constantBuffer, 0, NULL, constantBufferData, 0, 0);
	context.deviceContext->VSSetConstantBuffers(0, 1, constantBuffer);
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
			game_state gameState = {};
			gameState = Update();
			program.buffers.constantBufferData.pos = gameState.playerPos;
			program.buffers.constantBufferData.r = gameState.playerRot;
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			
			program.context.deviceContext->Draw(4, 0);
			
			program.context.swapChain->Present(1, 0);
		}
	} else {
		Win32ShowErrorBox("Failed to init Win32 and D3D");
	}
	return(0);
}
