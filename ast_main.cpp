#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

#define internal static
#define global static
#define persist static

#define true 1
#define false 0

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
			int unsigned running = true; //TODO: add b32
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