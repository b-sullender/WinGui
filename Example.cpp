
#include "WinGui.h"

#define WinClassName L"WinGui Example App"
#define WinTitle L"WinGui Example App"

static HWND mainWin_hWnd;
static HWND button1_hWnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC WinDC;
	PAINTSTRUCT ps;
	int wmId;
	int wmEvent;

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PRINTCLIENT:
	{
		WinDC = (HDC)wParam;

		// Code goes here ...

		return 0;
	}
	case WM_PAINT:
	{
		WinDC = BeginPaint(hWnd, &ps);

		// Code goes here ...

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmEvent)
		{
		case BN_CLICKED:
		{
			// Code goes here ...

			break;
		}
		default:
			break;
		}

		break;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ WCHAR* pCmdLine, _In_ int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;

	if (!WinGui_Init(hInstance, WINGUI_FLAGS_REPORT_ERRORS))
		return 0;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WinClassName;
	wcex.hIconSm = NULL;

	if (!WinGui_RegisterClassEx(&wcex))
	{
		MessageBox(0, L"Failed to register the main window class.", L"Error!", MB_OK);
		return 0;
	}

	mainWin_hWnd = WinGui_CreateWindow(0, WinClassName, WinTitle,
		WS_VISIBLE | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX,
		0, 0, 1000, 450, true, NULL, NULL, hInstance, NULL, NULL);

	if (!mainWin_hWnd)
	{
		MessageBox(0, L"Failed to create the main window.", L"Error!", MB_OK);
		return 0;
	}

	button1_hWnd = WinGui_CreateWindow(NULL, L"BUTTON", L"Button1", WS_VISIBLE | WS_CHILD,
		12, 12, 68, 24, false, mainWin_hWnd, NULL, hInstance, NULL, NULL);

	WinGui_SetAnchors(button1_hWnd, ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_RIGHT);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
