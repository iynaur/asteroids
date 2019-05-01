#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#pragma warning(pop)
#include "common_types.h"

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
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "WindowClass";
	if (RegisterClass(&wc)) {
		DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		HWND hwnd = CreateWindowA(wc.lpszClassName, "Asteroids", style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wc.hInstance, 0);
		if (hwnd) {
			HRESULT hr;
			
			RECT r;
			GetClientRect(hwnd, &r);
			u32 clientWidth = (u32)r.right - r.left;
			u32 clientHeight = (u32)r.bottom - r.top;
			
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
				DXGI_MODE_SCALING_UNSPECIFIED,
				1,
				hwnd,
				TRUE, //TODO: Windowed mode, implement fullscreen
				DXGI_SWAP_EFFECT_DISCARD,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			};
			
			IDXGISwapChain* swapChain;
			ID3D11Device* device;
			ID3D11DeviceContext* deviceContext;
			
			hr = D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				D3D11_CREATE_DEVICE_SINGLETHREADED, //TODO: Change if more threads which call d3d are activated
				NULL, //TODO: feature levels
				0, //NOTE: number of feature levels, change if feature levels are added
				D3D11_SDK_VERSION,
				&swapChainDesc,
				&swapChain,
				&device,
				NULL,
				&deviceContext
				);
			
			b32 running = true; //TODO: add b32
			while (running) {
				MSG msg;
				while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
					if (msg.message==WM_QUIT) running = false;
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				}
			}
		} else {
			//TODO: Error logging/messagebox 
		}
	} else {
		//TODO: Error logging/messagebox
	}
	return(0);
}