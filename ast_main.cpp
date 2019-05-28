/*
---------------- TODO LIST ----------------
- Fullscreen mode + switching with atl+enter
- Offline shader compilation
- Texture rendering?
- Clean up TODOs
- implement 1D noise instead of rand() for engine trail
- Implement custom rand function
- Draw all n vertex buffers
*/

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <math.h>
#pragma warning(pop)
#include "common_types.h"
#include "common_maths.h"
#include "win32_d3d.cpp"

struct program_state {
	b32 running;
};

#define ACTIVE_KEYS 4
struct controller_input {
	union {
		b32 keys[ACTIVE_KEYS];
		struct {
			b32 left;
			b32 up;
			b32 right;
			b32 down;
		};
	};
};

struct player_state {
	vec2 playerPos;
	float playerRot;
	float scale;
};

internal player_state UpdatePlayer(controller_input input)
{
	float maxSpeed = 1.0f;
	
	player_state result;
	persist player_state newState = {};
	newState.scale = 25.0f;
	
	if (input.right && !input.left) {
		newState.playerRot += 0.1f;
	} else if (!input.right && input.left) {
		newState.playerRot -= 0.1f;
	}
	vec2 aimVector = V2Normalise(RadToVec2(newState.playerRot));
	
	//TODO: Make drag a constant force, rather than only applied when not accelerating
	//TODO: Allow no drag when ship is in a vacuum
	
	persist vec2 velocity;
	float accelRate = 0.01f;
	float decelRate = 0.003f;
	float velocityMag = V2Mag(velocity);
	if (input.up) {
		velocity = (aimVector*accelRate) + velocity;
		if (velocityMag > maxSpeed) {
			velocity = V2Normalise(velocity) * maxSpeed;
		}
	} else {
		float newMag = fClamp(velocityMag - decelRate, 0.0f, velocityMag);
		velocity = V2Normalise(velocity) * newMag;
	}
	newState.playerPos = newState.playerPos - velocity;
	
	
	if (newState.playerPos.x > (1.0f * newState.scale*16.0f/9.0f)) {
		newState.playerPos.x = -1.0f * newState.scale *16.0f/9.0f;
	}
	if (newState.playerPos.x < (-1.0f * newState.scale*16.0f/9.0f)) {
		newState.playerPos.x = 1.0f * newState.scale *16.0f/9.0f;
	}
	
	if (newState.playerPos.y > (1.0f * newState.scale)) {
		newState.playerPos.y = -1.0f * newState.scale;
	}
	if (newState.playerPos.y < (-1.0f * newState.scale)) {
		newState.playerPos.y = 1.0f * newState.scale;
	}
	
	result.playerPos = newState.playerPos;
	result.playerRot = newState.playerRot;
	result.scale = newState.scale;
	return(result);
}

internal void UpdateConstBuffers(d3d_context context, ID3D11Buffer** constantBuffer, void* constantBufferData)
{
	context.deviceContext->UpdateSubresource(*constantBuffer, 0, NULL, constantBufferData, 0, 0);
	context.deviceContext->VSSetConstantBuffers(0, 1, constantBuffer);
}

internal void SetProgramVSConstantBuffer(win32_d3d_program* program, vs_constant_buffer values)
{
	int s = sizeof(values);
	for (int i = 0; i < s; ++i) {
		((byte*)&program->buffers.constantBufferData)[i] = ((byte*)&values)[i];
	}
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
				WPARAM i = msg.wParam - VK_LEFT;
				if (i < ACTIVE_KEYS) {
					input->keys[i] = (b32)(msg.message==WM_KEYDOWN);
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
	if (Win32D3DInitEverything(&program, MainWindowProc, 1280, 720)) {
		program_state programState = { true };
		controller_input controllerInput = {};
		player_state playerState = { 0.0f, 0.0f, 0.0f, 10.0f};
		while (programState.running) {
			Win32GetControllerInput(&controllerInput, &programState);
			
			float clearColour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			program.context.deviceContext->ClearRenderTargetView(program.context.renderTargetView, clearColour);
			playerState = UpdatePlayer(controllerInput);
			
			
			program.buffers.constantBufferData = {};
			vs_constant_buffer square = {};
			square.cameraPos = playerState.playerPos;
			square.r = Win32GetTime();
			square.scale = playerState.scale;
			square.ar = program.buffers.ar;
			square.localOffset = {};
			square.pos = {};
			square.distort.x = 1.0f;
			square.distort.y = 1.0f;
			SetProgramVSConstantBuffer(&program, square);
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			
			UINT stride = sizeof(vec2);
			UINT offset = 0;
			program.context.deviceContext->IASetVertexBuffers(0, 1, &program.buffers.vertexBuffers[2].buffer, &stride, &offset);
			program.context.deviceContext->Draw(4, 0);
			
			
			vs_constant_buffer playerShip = square;
			playerShip.localOffset = {};
			playerShip.pos = playerState.playerPos;
			playerShip.r = playerState.playerRot;
			playerShip.scale = playerState.scale;
			playerShip.distort.x = 1.0f;
			playerShip.distort.y = 1.0f;
			SetProgramVSConstantBuffer(&program, playerShip);
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			
			program.context.deviceContext->IASetVertexBuffers(0, 1, &program.buffers.vertexBuffers[0].buffer, &stride, &offset);
			program.context.deviceContext->Draw(4, 0);
			
			if (controllerInput.up) {
				vs_constant_buffer playerShipTrail = playerShip;
				playerShipTrail.distort.width = 1.0f - (RandomFloat() / 2.0f);
				playerShipTrail.distort.height = 1.0f - (RandomFloat() / 4.0f);
				playerShipTrail.localOffset.y = -1.0f;
				SetProgramVSConstantBuffer(&program, playerShipTrail);
				UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
				program.context.deviceContext->IASetVertexBuffers(0, 1, &program.buffers.vertexBuffers[1].buffer, &stride, &offset);
				program.context.deviceContext->Draw(4, 0);
			}
			
			program.context.swapChain->Present(1, 0);
		}
	} else {
		Win32ShowErrorBox("Failed to init Win32 and D3D");
	}
	return(0);
}
