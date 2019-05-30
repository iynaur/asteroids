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

struct entity_state {
	vec2 pos;
	vec2 distort;
	vec2 localOffset;
	float rot;
};

struct game_state {
	vec2 cameraPos;
	float cameraScale;
	float aspectRatio;
};

internal entity_state UpdatePlayer(controller_input input, game_state gameState)
{
	float maxSpeed = 1.0f;
	
	entity_state result;
	persist entity_state newState = { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
	
	if (input.right && !input.left) {
		newState.rot += 0.1f;
	} else if (!input.right && input.left) {
		newState.rot -= 0.1f;
	}
	vec2 aimVector = V2Normalise(RadToVec2(newState.rot));
	
	//TODO: Make drag a constant force, rather than only applied when not accelerating
	//TODO: Allow for no drag when ship is in a vacuum
	
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
	newState.pos = newState.pos - velocity;
	
	
	vec2 axisCamScale = { gameState.cameraScale * (1/gameState.aspectRatio), gameState.cameraScale };
	if (newState.pos.x > (1.0f * axisCamScale.x)) {
		newState.pos.x = -1.0f * axisCamScale.x;
	}
	if (newState.pos.x < (-1.0f * axisCamScale.x)) {
		newState.pos.x = 1.0f * axisCamScale.x;
	}
	
	if (newState.pos.y > (1.0f * axisCamScale.y)) {
		newState.pos.y = -1.0f * axisCamScale.y;
	}
	if (newState.pos.y < (-1.0f * axisCamScale.y)) {
		newState.pos.y = 1.0f * axisCamScale.y;
	}
	
	result = newState;
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

struct vs_constant_pointer_buffer {
	vec2* pos;
	vec2* cameraPos;
	vec2* distort;
	vec2* localOffset;
	float* ar;
	float* scale;
	float* r;
};

internal void SetVSBufferFromPointerBuffer(vs_constant_buffer* buffer, vs_constant_pointer_buffer pointers)
{
	buffer->pos = *pointers.pos;
	buffer->cameraPos = *pointers.cameraPos;
	buffer->distort = *pointers.distort;
	buffer->localOffset = *pointers.localOffset;
	buffer->ar = *pointers.ar;
	buffer->scale = *pointers.scale;
	buffer->r = *pointers.r;
}

#pragma warning(push, 0)
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
#pragma warning(pop)
{
	win32_d3d_program program;
	if (Win32D3DInitEverything(&program, MainWindowProc, 1280, 720)) {
		program_state programState = { true };
		controller_input controllerInput = {};
		game_state gameState;
		gameState.cameraPos = {0.0f, 0.0f};
		gameState.cameraScale = 25.0f;
		gameState.aspectRatio = program.buffers.ar;
		
		
		entity_state entities[3];
		
		
		entities[0].pos = {0.0f, 0.0f};
		entities[0].distort = {1.0f, 1.0f};
		entities[0].localOffset = {0.0f, 0.0f};
		entities[0].rot = 0.0f;
		
		entities[1].pos = {0.0f, 0.0f};
		entities[1].distort = {1.0f, 1.0f};
		entities[1].localOffset = {0.0f, -1.0f};
		entities[1].rot = 0.0f;
		
		entities[2].pos = {0.0f, 0.0f};
		entities[2].distort = {1.0f, 1.0f};
		entities[2].localOffset = {0.0f, 0.0f};
		entities[2].rot = 0.0f;
		
		
		vs_constant_pointer_buffer entityConstantPointerBuffers[3];
		for (int i = 0; i < 3; ++i) {
			entityConstantPointerBuffers[i].pos = &entities[i].pos;
			entityConstantPointerBuffers[i].cameraPos = &gameState.cameraPos;
			entityConstantPointerBuffers[i].distort = &entities[i].distort;
			entityConstantPointerBuffers[i].localOffset = &entities[i].localOffset;
			entityConstantPointerBuffers[i].ar = &gameState.aspectRatio;
			entityConstantPointerBuffers[i].scale = &gameState.cameraScale;
			entityConstantPointerBuffers[i].r = &entities[i].rot;
		}
		
		vs_constant_buffer entityConstantBuffers[3];
		
		while (programState.running) {
			Win32GetControllerInput(&controllerInput, &programState);
			float clearColour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			program.context.deviceContext->ClearRenderTargetView(program.context.renderTargetView, clearColour);
			//TODO: Update player should return entitiy_state, get rid of player_state
			entities[0] = UpdatePlayer(controllerInput, gameState);
			gameState.cameraPos = entities[0].pos;
			
			program.buffers.constantBufferData = {};
			SetVSBufferFromPointerBuffer(&entityConstantBuffers[2], entityConstantPointerBuffers[2]);
			SetProgramVSConstantBuffer(&program, entityConstantBuffers[2]);
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			
			UINT stride = sizeof(vec2);
			UINT offset = 0;
			program.context.deviceContext->IASetVertexBuffers(0, 1, &program.buffers.vertexBuffers[2].buffer, &stride, &offset);
			program.context.deviceContext->Draw(4, 0);
			
			program.buffers.constantBufferData = {};
			SetVSBufferFromPointerBuffer(&entityConstantBuffers[0], entityConstantPointerBuffers[0]);
			SetProgramVSConstantBuffer(&program, entityConstantBuffers[0]);
			UpdateConstBuffers(program.context, &program.buffers.constantBuffer, &program.buffers.constantBufferData);
			program.context.deviceContext->IASetVertexBuffers(0, 1, &program.buffers.vertexBuffers[0].buffer, &stride, &offset);
			program.context.deviceContext->Draw(4, 0);
			
			if (controllerInput.up) {
				program.buffers.constantBufferData = {};
				entities[1].pos = entities[0].pos;
				entities[1].rot = entities[0].rot;
				entities[1].distort.width = 1.0f - (RandomFloat() / 2.0f);
				entities[1].distort.height = 1.0f - (RandomFloat() / 4.0f);
				SetVSBufferFromPointerBuffer(&entityConstantBuffers[1], entityConstantPointerBuffers[1]);
				SetProgramVSConstantBuffer(&program, entityConstantBuffers[1]);
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
