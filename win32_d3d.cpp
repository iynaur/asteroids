#ifndef WIN32_D3D_CPP
#define WIN32_D3D_CPP

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

internal void Win32ShowErrorBox(char* message)
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

internal void UpdateConstBuffers(d3d_context context, ID3D11Buffer** constantBuffer, void* constantBufferData)
{
	context.deviceContext->UpdateSubresource(*constantBuffer, 0, NULL, constantBufferData, 0, 0);
	context.deviceContext->VSSetConstantBuffers(0, 1, constantBuffer);
}

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
		//TODO: check if all functions succeed here
		LARGE_INTEGER fileSize;
		GetFileSizeEx(fileHandle, &fileSize);
		u64 size = (u64)fileSize.QuadPart;
		void* shaderCode = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		DWORD readBytes = 0;
		ReadFile(fileHandle, shaderCode, size, &readBytes, NULL);
		//TODO: error msgs
		HRESULT hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"VS", "vs_5_0", NULL, NULL, &shaders->vertexShaderBlob, NULL);
		if (SUCCEEDED(hr)) {
			hr = D3DCompile(shaderCode, size, NULL, NULL, NULL,"PS", "ps_5_0", NULL, NULL, &pixelShaderBlob, NULL);
			if (SUCCEEDED(hr)) {
				device->CreateVertexShader(shaders->vertexShaderBlob->GetBufferPointer(), shaders->vertexShaderBlob->GetBufferSize(), NULL, &vertexShader);
				device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &pixelShader);
				deviceContext->VSSetShader(vertexShader, NULL, 0);
				deviceContext->PSSetShader(pixelShader, NULL, 0);
				result = true;
			} else {
				Win32ShowErrorBox("Failed to compile pixel shader");
			}
		} else {
			Win32ShowErrorBox("Failed to compile vertex shader");
		}
	} else {
		Win32Error("Failed to open shader file");
	}
	return(result);
}

struct d3d_buffers {
	ID3D11Buffer* constantBuffer;
	struct vs_constant_buffer {
		float ar;
		float scale;
		vec2 pos;
	} constantBufferData;
};

internal b32 D3DInitBuffers(d3d_buffers* buffers, d3d_context context, d3d_shaders shaders, rect clientDimensions)
{
	b32 result = false;
	HRESULT hr;
	
	ID3D11Buffer* triVertexBuffer;
	
	vec2 v[] = {
		{ -0.75f, -1.0f },
		{ 0.0f, 1.0f },
		{ 0.0f, -0.75f },
		{ 0.75f, -1.0f },
	};
	
	D3D11_BUFFER_DESC vertexBufferDesc = {
		sizeof(v),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA vertexBufferData = {
		v,
		0,
		0
	};
	hr = context.device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triVertexBuffer); 
	if (SUCCEEDED(hr)) {
		UINT stride = sizeof(vec2);
		UINT offset = 0;
		context.deviceContext->IASetVertexBuffers(0, 1, &triVertexBuffer, &stride, &offset);
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
			
			buffers->constantBufferData.ar = clientDimensions.height / clientDimensions.width;
			buffers->constantBufferData.scale = 25.0f;
			buffers->constantBufferData.pos = { 0.0f, 0.0f };
			
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
		Win32ShowErrorBox("Failed to create buffer");
	}
	return(result);
}

struct win32_d3d_program {
	d3d_context context;
	d3d_shaders shaders;
	d3d_buffers buffers;
};

//TODO: allow passing of program settings such as resolution
internal b32 Win32D3DInitEverything(win32_d3d_program* program, WNDPROC proc)
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
		HWND hwnd = CreateWindowA(wc.lpszClassName, "Asteroids", style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wc.hInstance, 0);
		if (hwnd) {
			RECT r;
			if (GetClientRect(hwnd, &r)) {
				u32 clientWidth = (u32)r.right - r.left;
				u32 clientHeight = (u32)r.bottom - r.top;
				if (D3DInitContext(&program->context, hwnd, clientWidth, clientHeight)) {
					if (D3DInitShaders(&program->shaders, program->context.device, program->context.deviceContext)) {
						rect clientDimensions = {(float)clientWidth, (float)clientHeight};
						if (D3DInitBuffers(&program->buffers, program->context, program->shaders, clientDimensions)) {
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