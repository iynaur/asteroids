#ifndef WIN32_D3D_CPP
#define WIN32_D3D_CPP

#include "common_types.h"

inline __int64 Win32GetTicks(void)
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return(time.QuadPart);
}

inline __int64 Win32GetFreq(void)
{
	LARGE_INTEGER time;
	QueryPerformanceFrequency(&time);
	return(time.QuadPart);
}

inline float Win32GetTime(void)
{
	return((float)Win32GetTicks() / (float)Win32GetFreq());
}

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

internal void Win32ShowErrorBox(char* message)
{
	MessageBoxA(0, message, "Error", MB_ICONERROR);
}


internal void* Win32AllocateWritableMemory(SIZE_T size)
{
	void* result = (void*)0;
	void* memory = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (memory) {
		result = memory;
	} else {
		Win32Error("Failed to allocate memory");
	}
	return(result);
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
		DXGI_MODE_SCALING_UNSPECIFIED
	};
	
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
		modeDesc,
		{ 8, 0 },
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		1,
		hwnd,
		TRUE,
		DXGI_SWAP_EFFECT_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		D3D11_CREATE_DEVICE_SINGLETHREADED, //NOTE: Change if more threads which call d3d are activated
		0, //TODO: feature levels
		0, //NOTE: number of feature levels, change if feature levels are added
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&context->swapChain,
		&context->device,
		0,
		&context->deviceContext
		);
	if (SUCCEEDED(hr)) {
		hr = context->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&context->backBufferTexture);
		if (SUCCEEDED(hr)) {
			hr = context->device->CreateRenderTargetView(context->backBufferTexture, 0, &context->renderTargetView);
			if (SUCCEEDED(hr)) {
				context->backBufferTexture->Release();
				context->deviceContext->OMSetRenderTargets(1, &context->renderTargetView, 0);
				result = true;
			} else {
				Win32ShowErrorBox("Failed to create render target view");
			}
		} else {
			Win32ShowErrorBox("Failed to get buffer");
		}
	} else {
		Win32ShowErrorBox("Failed to create Device and swap chain");
	}
	return(result);
}

struct d3d_shaders {
	ID3D10Blob* vertexShaderBlob;
};

internal b32 D3DInitShaders(d3d_shaders* shaders, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	b32 result = false;
	ID3D10Blob* pixelShaderBlob;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	
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
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			u64 size = (u64)fileSize.QuadPart;
			void* shaderCode = Win32AllocateWritableMemory(size); //TODO: Load into memory pool
			if (shaderCode != NULL) {
				
				DWORD readBytes = 0;
				if (ReadFile(fileHandle, shaderCode, size, &readBytes, NULL)) {
					ID3DBlob* errorMsg;
					HRESULT hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"VS", "vs_5_0", D3DCOMPILE_DEBUG, NULL, &shaders->vertexShaderBlob, &errorMsg);
					if (SUCCEEDED(hr)) {
						hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"PS", "ps_5_0", D3DCOMPILE_DEBUG, NULL, &pixelShaderBlob, &errorMsg);
						if (SUCCEEDED(hr)) {
							device->CreateVertexShader(shaders->vertexShaderBlob->GetBufferPointer(), shaders->vertexShaderBlob->GetBufferSize(), NULL, &vertexShader);
							device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &pixelShader);
							deviceContext->VSSetShader(vertexShader, NULL, 0);
							deviceContext->PSSetShader(pixelShader, NULL, 0);
							result = true;
						} else {
							Win32ShowErrorBox("Failed to compile pixel shader");
							Win32ShowErrorBox((char*)errorMsg->GetBufferPointer());
						}
					} else {
						Win32ShowErrorBox("Failed to compile vertex shader");
						Win32ShowErrorBox((char*)errorMsg->GetBufferPointer());
					}
				} else {
					Win32Error("Failed to read file");
				}
			} else {
				Win32Error("Failed to allocate memory");
			}
		} else {
			Win32Error("Failed to get file size");
		}
	} else {
		Win32Error("Failed to open shader file");
	}
	return(result);
}

struct d3d_buffer {
	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA bufferData;
};

struct vs_constant_buffer {
	vec2 pos;
	vec2 cameraPos;
	vec2 distort;
	vec2 localOffset;
	float ar;
	float scale;
	float r;
};

struct d3d_program_buffers {
	ID3D11Buffer* constantBuffer;
	d3d_buffer* vertexBuffers;
	float ar;
	vs_constant_buffer constantBufferData;
};

internal b32 D3DInitBuffer(d3d_buffer* buffer, d3d_context context)
{
	b32 result = false;
	
	HRESULT hr = context.device->CreateBuffer(&buffer->bufferDesc, &buffer->bufferData, &buffer->buffer); 
	if (SUCCEEDED(hr)) {
		result = true;
	} else {
		Win32ShowErrorBox("Failed to create buffer");
	}
	return(result);
}

internal b32 D3DInitVertexBuffers(d3d_program_buffers* buffers, d3d_context context, int n)
{
	b32 result = false;
	
	int bufferWidth = 4;
	buffers->vertexBuffers = (d3d_buffer*)Win32AllocateWritableMemory(n*bufferWidth*sizeof(vec2));
	//TODO: load data from disk
	vec2 v[3*4] = {
		{ -0.75f, -1.0f },
		{ 0.0f, 1.0f },
		{ 0.0f, -0.75f },
		{ 0.75f, -1.0f },
		
		{ -0.25f, -0.25f },
		{ 0.0f, -0.0f },
		{ 0.0f, -1.5f },
		{ 0.25f, -0.25f },
		
		{ -1.1f, 1.2f },
		{ 1.3f, 1.4f },
		{ -1.5f, -1.6f },
		//{ 1.0f, -1.0f },
	};
	
	//TODO: allow for n sided polygons
	for (int i = 0; i < n; ++i) {
		buffers->vertexBuffers[i].bufferDesc = {};
		buffers->vertexBuffers[i].bufferDesc.ByteWidth = sizeof(v);
		buffers->vertexBuffers[i].bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffers->vertexBuffers[i].bufferData = {&v[i*bufferWidth], 0, 0};
		if (D3DInitBuffer(&buffers->vertexBuffers[i], context)) {
			if (i == n-1) {
				result = true;
			}
		} else {
			Win32ShowErrorBox("Failed to init buffer");
			break;
		}
	}
	return(result);
}

internal b32 D3DInitBuffers(d3d_program_buffers* buffers, d3d_context context, d3d_shaders shaders, rect clientDimensions, int n)
{
	b32 result = false;
	HRESULT hr;
	
	if (D3DInitVertexBuffers(buffers, context, n)) {
		ID3D11InputLayout* vertexLayout;
		D3D11_INPUT_ELEMENT_DESC layout = {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		};
		hr = context.device->CreateInputLayout(&layout, 1, shaders.vertexShaderBlob->GetBufferPointer(), shaders.vertexShaderBlob->GetBufferSize(), &vertexLayout);
		if (SUCCEEDED(hr)){
			context.deviceContext->IASetInputLayout(vertexLayout);
			context.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			
			D3D11_VIEWPORT viewport = {
				0, 
				0, 
				clientDimensions.width,
				clientDimensions.height,
				0.0f,
				1.0f,
			};
			context.deviceContext->RSSetViewports(1, &viewport);
			
			buffers->ar = clientDimensions.height / clientDimensions.width;
			buffers->constantBufferData.ar = buffers->ar;
			buffers->constantBufferData.scale = 25.0f;
			buffers->constantBufferData.pos = { 0.0f, 0.0f };
			buffers->constantBufferData.r = 0.0f;
			
			D3D11_BUFFER_DESC constantBufferDesc = {};
			constantBufferDesc.ByteWidth = sizeof(buffers->constantBufferData);
			constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			constantBufferDesc.BindFlags = 0;
			constantBufferDesc.CPUAccessFlags = 0;
			constantBufferDesc.MiscFlags = 0;
			constantBufferDesc.StructureByteStride = 0;
			hr = context.device->CreateBuffer(&constantBufferDesc, NULL, &buffers->constantBuffer);
			if (SUCCEEDED(hr)) {
				result = true;
			} else {
				Win32ShowErrorBox("Failed to create constant buffer");
			}
		} else {
			Win32ShowErrorBox("Failed to create input layout");
		}
	} else {
		Win32ShowErrorBox("Failed to init buffers"); //TODO: print which buffers failed to init
	}
	return(result);
}

struct win32_d3d_program {
	d3d_context context;
	d3d_shaders shaders;
	d3d_program_buffers buffers;
};

internal b32 Win32D3DInitEverything(win32_d3d_program* program, WNDPROC proc, int windowWidth, int windowHeight)
{
	b32 result = false;
	WNDCLASSA wc = {};
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = 0;
	wc.lpszClassName = "WindowClass";
	if (RegisterClass(&wc)) {
		DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		RECT adjustRect = { 0, 0, windowWidth, windowHeight };
		AdjustWindowRect(&adjustRect, style, FALSE);
		HWND hwnd = CreateWindowA(wc.lpszClassName, "Asteroids", style, CW_USEDEFAULT, CW_USEDEFAULT, adjustRect.right - adjustRect.left, adjustRect.bottom - adjustRect.top, 0, 0, wc.hInstance, 0);
		if (hwnd) {
			RECT r;
			if (GetClientRect(hwnd, &r)) {
				u32 clientWidth = (u32)r.right - r.left;
				u32 clientHeight = (u32)r.bottom - r.top;
				if (D3DInitContext(&program->context, hwnd, clientWidth, clientHeight)) {
					if (D3DInitShaders(&program->shaders, program->context.device, program->context.deviceContext)) {
						rect clientDimensions = {(float)clientWidth, (float)clientHeight};
						if (D3DInitBuffers(&program->buffers, program->context, program->shaders, clientDimensions, 3)) {
							result = true;
						} else {
							Win32ShowErrorBox("Failed to init buffers");
						}
					} else {
						Win32ShowErrorBox("Failed to init shaders");
					}
				} else {
					Win32ShowErrorBox("Failed to init d3d context");
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
	return(result);
}


#endif 