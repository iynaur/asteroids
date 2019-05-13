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

struct program_state {
	b32 running;
};


struct controller_input {
	b32 up;
	b32 down;
	b32 left;
	b32 right;
};

struct game_state {
	vec2 playerPos;
	float playerRot;
};

internal game_state Update(controller_input input)
{
	game_state result;
	persist vec2 pos = {10.0f, 10.0f};
	persist float r = 0.0f;
	if (input.up && !input.down) {
		pos.y -= 0.1f;
	} else if (!input.up && input.down) {
		pos.y += 0.1f;
	}
	
	if (input.right && !input.left) {
		pos.x -= 0.1f;
	} else if (!input.right && input.left) {
		pos.x += 0.1f;
	}
	
	result.playerPos = pos;
	result.playerRot = r;
	return(result);
}

internal void UpdateConstBuffers(d3d_context context, ID3D11Buffer** constantBuffer, void* constantBufferData)
{
	context.deviceContext->UpdateSubresource(*constantBuffer, 0, NULL, constantBufferData, 0, 0);
	context.deviceContext->VSSetConstantBuffers(0, 1, constantBuffer);
}

internal void Win32GetControllerInput(controller_input* input, program_state* programState)
{
	MSG msg;
	while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
		switch(msg.message) {
			case WM_QUIT: {
				programState->running = false;
			} break;
			case WM_KEYUP:
			case WM_KEYDOWN: {
				switch(msg.wParam) {
					case VK_UP: {
						input->up = (b32)(msg.message==WM_KEYDOWN);
					} break;
					case VK_DOWN: {
						input->down = (b32)(msg.message==WM_KEYDOWN);
					} break;
					case VK_LEFT: {
						input->left = (b32)(msg.message==WM_KEYDOWN);
					} break;
					case VK_RIGHT: {
						input->right = (b32)(msg.message==WM_KEYDOWN);
					} break;
				}
			} break;
			default: {
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			} break;
		}
	}
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
		program_state programState = { true };
		controller_input controllerInput = {};
		while (programState.running) {
			Win32GetControllerInput(&controllerInput, &programState);
			
			float clearColour[4] = {0.0f, 0.2f, 0.4f, 1.0f};
			program.context.deviceContext->ClearRenderTargetView(program.context.renderTargetView, clearColour);
			game_state gameState = {};
			gameState = Update(controllerInput);
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
