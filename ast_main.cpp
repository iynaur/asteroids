/*
---------------- TODO LIST ----------------
- Fullscreen mode + switching with atl+enter
- Offline shader compilation
*/

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)
#include "common_types.h"

internal void Win32Error(char* customErrorMessage)
{
	DWORD err =  GetLastError();
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	LPTSTR lpBuffer = 0;
	FormatMessage(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpBuffer, 0, 0);
	if (lpBuffer) {
		MessageBoxA(0, lpBuffer, customErrorMessage, MB_ICONERROR);
		LocalFree(lpBuffer);
		lpBuffer = 0;
	}
}

internal void Win32ShowMessageBoxError(char* message)
{
	MessageBoxA(0, message, "Error", MB_ICONERROR);
}

#pragma pack(push, 1)
struct d3d_context {
	IDXGISwapChain* swapChain;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* backBufferTexture;
};
#pragma pack(pop)

internal b32 D3DInitContext(d3d_context* context, HWND hwnd, u32 clientWidth, u32 clientHeight)
{
	b32 result = false;
	DXGI_RATIONAL refreshRate = { 1, 60 };
	
	DXGI_MODE_DESC modeDesc = {
		clientWidth,
		clientHeight,
		refreshRate,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		DXGI_MODE_SCALING_UNSPECIFIED // NOTE: Scaling, possibly change
	};
	
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
		modeDesc,
		{ 1, 0 },
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		1,
		hwnd,
		TRUE, //TODO: Windowed mode, implement fullscreen
		DXGI_SWAP_EFFECT_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		D3D11_CREATE_DEVICE_SINGLETHREADED, //TODO: Change if more threads which call d3d are activated
		0, //TODO: feature levels
		0, //NOTE: number of feature levels, change if feature levels are added
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&context->swapChain,
		&context->device,
		0,
		&context->deviceContext
		);
	//TODO: Error checking
	if (SUCCEEDED(hr)) {
		hr = context->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&context->backBufferTexture);
		if (SUCCEEDED(hr)) {
			hr = context->device->CreateRenderTargetView(context->backBufferTexture, 0, &context->renderTargetView);
			if (SUCCEEDED(hr)) {
				context->backBufferTexture->Release();
				context->deviceContext->OMSetRenderTargets(1, &context->renderTargetView, 0);
				result = true;
			} else {
				Win32ShowMessageBoxError("Failed to create render target view");
			}
		} else {
			Win32ShowMessageBoxError("Failed to get buffer");
		}
	} else {
		Win32ShowMessageBoxError("Failed to create Device and swap chain");
	}
	return(result);
}

internal b32 D3DInitShaders(void)
{
	b32 result = false;
	ID3D10Blob *vertexShader, *pixelShader;
	
	char* shaderFilename = "../../code/shaders/shader.hlsl";
	
	HANDLE fileHandle = CreateFileA(
		shaderFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		//TODO: check if all functions succeed here
		LARGE_INTEGER fileSize;
		GetFileSizeEx(fileHandle, &fileSize);
		u64 size = (u64)fileSize.QuadPart;
		void* shaderCode = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		DWORD readBytes = 0;
		ReadFile(fileHandle, shaderCode, size, &readBytes, NULL);
		//TODO: error msgs
		HRESULT hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"VS", "vs_5_0", NULL, NULL, &vertexShader, NULL);
		if (SUCCEEDED(hr)) {
			hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"PS", "ps_5_0", NULL, NULL, &pixelShader, NULL);
			if (SUCCEEDED(hr)) {
				result = true;
			} else {
				Win32ShowMessageBoxError("Failed to compile pixel shader");
			}
		} else {
			Win32ShowMessageBoxError("Failed to compile vertex shader");
		}
	} else {
		Win32Error("Failed to open shader file");
	}
	return(result);
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
	WNDCLASSA wc = {};
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = 0;
	wc.lpszClassName = "WindowClass";
	if (RegisterClass(&wc)) {
		DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		HWND hwnd = CreateWindowA(wc.lpszClassName, "Asteroids", style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wc.hInstance, 0);
		if (hwnd) {
			RECT r;
			if (GetClientRect(hwnd, &r)) {
				u32 clientWidth = (u32)r.right - r.left;
				u32 clientHeight = (u32)r.bottom - r.top;
				d3d_context context;
				if (D3DInitContext(&context, hwnd, clientWidth, clientHeight)) {
					if (D3DInitShaders()) {
						b32 running = true;
						while (running) {
							MSG msg;
							while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
								if (msg.message==WM_QUIT) running = false;
								TranslateMessage(&msg);
								DispatchMessageA(&msg);
							}
							
							float clearColour[4] = {0.0f, 0.2f, 0.4f, 1.0f};
							context.deviceContext->ClearRenderTargetView(context.renderTargetView, clearColour);
							
							context.swapChain->Present(1, 0);
						}
					} else {
						Win32ShowMessageBoxError("Failed to init shaders");
					}
				} else {
					Win32ShowMessageBoxError("Failed to init d3d context");
				}
			} else {
				Win32Error("Failed to get client rect");
			}
		} else {
			Win32Error("Failed to create window");
		}
	} else {
		Win32Error("Could not register class");
	}
	return(0);
}