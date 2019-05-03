/*

---------------- TODO LIST ----------------
- Fullscreen mode + switching with atl+enter
- Offline shader compilation
*/

#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h> //TODO: remove
#include <d3dx10.h> //TODO: remove
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

internal void Win32MessageBoxError(char* message)
{
	MessageBoxA(0, message, "Error", MB_ICONERROR);
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
			HRESULT hr;
			
			RECT r;
			if (GetClientRect(hwnd, &r)) {
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
					DXGI_USAGE_RENDER_TARGET_OUTPUT,
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
					0,
					D3D_DRIVER_TYPE_HARDWARE,
					0,
					D3D11_CREATE_DEVICE_SINGLETHREADED, //TODO: Change if more threads which call d3d are activated
					0, //TODO: feature levels
					0, //NOTE: number of feature levels, change if feature levels are added
					D3D11_SDK_VERSION,
					&swapChainDesc,
					&swapChain,
					&device,
					0,
					&deviceContext
					);
				//TODO: Error checking
				if (SUCCEEDED(hr)) {
					ID3D11RenderTargetView* renderTargetView;
					ID3D11Texture2D* backBufferTexture;
					hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
					if (SUCCEEDED(hr)) {
						hr = device->CreateRenderTargetView(backBufferTexture, 0, &renderTargetView);
						if (SUCCEEDED(hr)) {
							backBufferTexture->Release();
							deviceContext->OMSetRenderTargets(1, &renderTargetView, 0);
							
							ID3D10Blob *vertexShader, *pixelShader;
							
							//TODO: switch to D3DCompile()
							hr = D3DX11CompileFromFile("../../code/shaders/shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, NULL, NULL, &vertexShader, NULL, NULL);
							
							if (SUCCEEDED(hr)) {
								hr = D3DX11CompileFromFile("../../code/shaders/shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, NULL, NULL, &pixelShader, NULL, NULL);
								if (SUCCEEDED(hr)) {
									b32 running = true;
									while (running) {
										MSG msg;
										while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
											if (msg.message==WM_QUIT) running = false;
											TranslateMessage(&msg);
											DispatchMessageA(&msg);
										}
										
										float clearColour[4] = {0.0f, 0.2f, 0.4f, 1.0f};
										deviceContext->ClearRenderTargetView(renderTargetView, clearColour);
										
										swapChain->Present(1, 0);
									}
								} else {
									Win32MessageBoxError("Failed to compile pixel shader");
								}
							} else {
								Win32MessageBoxError("Failed to compile vertex shader");
							}
						} else {
							Win32MessageBoxError("Failed to create render target view");
						}
					} else {
						Win32MessageBoxError("Failed to get buffer");
					}
				} else {
					Win32MessageBoxError("Failed to create Device and swap chain");
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