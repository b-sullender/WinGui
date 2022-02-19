
#include "WinGui.h"

// Common controls dependencies.

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// hInstance

static HINSTANCE winGui_hInstance;

// DPI function pointers.

static GetDpiForSystem_proc pGetDpiForSystem;
static GetDpiForWindow_proc pGetDpiForWindow;
static SetProcessDPIAware_proc pSetProcessDPIAware;
static SetProcessDpiAwareness_proc pSetProcessDpiAwareness;
static SetProcessDpiAwarenessContext_proc pSetProcessDpiAwarenessContext;
static EnableNonClientDpiScaling_proc pEnableNonClientDpiScaling;
static AdjustWindowRectExForDpi_proc pAdjustWindowRectExForDpi;

static HANDLE mutexLock = 0;
static WINGUI_CLASS_INFO* pWinGui_FirstClass = 0;

double dpi_MulDiv(double nNumber, double nNumerator, double nDenominator)
{
	return (nNumber * nNumerator) / nDenominator;
}

double dpi_PixelsToDips(double pixels, double monitorDpi)
{
	return pixels / (monitorDpi / GLOBAL_APPLICATION_DPI);
}

void dpi_ResizeFont(HWND hWnd, int monitorDpi, DPI_VIRTUAL_INFO* virDpi)
{
	HFONT hFontOld;
	HFONT hFontNew;
	LOGFONT font;

	font = {};
	hFontOld = GetWindowFont(hWnd);

	if (virDpi->lpFont != 0)
	{
		memcpy(&font, virDpi->lpFont, sizeof(LOGFONT));
		font.lfHeight = (LONG)-dpi_MulDiv(virDpi->lpFont->lfHeight, monitorDpi, GLOBAL_APPLICATION_DPI);
		hFontNew = CreateFontIndirect(&font);
		if (hFontNew)
		{
			DeleteObject(hFontOld);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hFontNew, MAKELPARAM(TRUE, 0));
		}
	}
}

void winGui_GetdefaultFont(LOGFONT* lpFont, int AppDPI)
{
	lpFont->lfWeight = 400;
	lpFont->lfCharSet = 1;
	lpFont->lfQuality = 5;
	wcscpy_s(lpFont->lfFaceName, 32, L"Segoe UI");
	lpFont->lfHeight = (LONG)dpi_MulDiv(12, (double)AppDPI, 96);
}

void winGui_AddClass(WINGUI_CLASS_INFO* pWinGui_Class)
{
	WaitForSingleObject(mutexLock, INFINITE);

	if (pWinGui_FirstClass == 0)
		pWinGui_FirstClass = pWinGui_Class;
	else
	{
		pWinGui_Class->next = pWinGui_FirstClass;
		pWinGui_FirstClass = pWinGui_Class;
	}

	ReleaseMutex(mutexLock);
}

bool winGui_RemoveClass(const WCHAR* pClassName)
{
	bool result;
	WINGUI_CLASS_INFO* pThisClass, * pPrevClass;

	result = false;
	pPrevClass = 0;

	WaitForSingleObject(mutexLock, INFINITE);

	if (pWinGui_FirstClass != 0)
	{
		if (wcscmp(pClassName, pWinGui_FirstClass->className) == 0)
		{
			pWinGui_FirstClass = pWinGui_FirstClass->next;
			goto removeClass_exit;
		}

		pPrevClass = pWinGui_FirstClass;
		pThisClass = pWinGui_FirstClass->next;
		while (pThisClass != 0)
		{
			if (wcscmp(pClassName, pThisClass->className) == 0)
			{
				if (pPrevClass)
					pPrevClass->next = pThisClass->next;

				break;
			}
			pPrevClass = pThisClass;
			pThisClass = pThisClass->next;
		}
	}

removeClass_exit:
	ReleaseMutex(mutexLock);

	return result;
}

WINGUI_CLASS_INFO* winGui_FindClass(const WCHAR* pClassName)
{
	WINGUI_CLASS_INFO* pThisClass, * result;

	WaitForSingleObject(mutexLock, INFINITE);

	result = 0;

	pThisClass = pWinGui_FirstClass;
	while (pThisClass != 0)
	{
		if (wcscmp(pClassName, pThisClass->className) == 0)
		{
			result = pThisClass;
			break;
		}
		pThisClass = pThisClass->next;
	}

	ReleaseMutex(mutexLock);

	return result;
}

bool WinGui_Init(HINSTANCE hInstance, unsigned long flags)
{
	INITCOMMONCONTROLSEX ICC;
	HMODULE hMod_user32, hMod_shcore;

	winGui_hInstance = hInstance;

	// Init DPI Awareness

	pGetDpiForSystem = NULL;
	pGetDpiForWindow = NULL;
	pSetProcessDPIAware = NULL;
	pSetProcessDpiAwareness = NULL;
	pSetProcessDpiAwarenessContext = NULL;
	pEnableNonClientDpiScaling = NULL;
	pAdjustWindowRectExForDpi = NULL;

	// Try to use SetProcessDpiAwarenessContext (Windows 10)

	hMod_user32 = GetModuleHandle(L"user32.dll");
	if (hMod_user32 == NULL)
	{
		hMod_user32 = LoadLibrary(L"user32.dll");
	}

	if (hMod_user32 != NULL)
	{
		pSetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_proc)GetProcAddress(hMod_user32, "SetProcessDpiAwarenessContext");
		if (pSetProcessDpiAwarenessContext != NULL)
		{
			if (pSetProcessDpiAwarenessContext(GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) goto DpiAwarenessSet;
			if (pSetProcessDpiAwarenessContext(GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) goto DpiAwarenessSet;
			if (pSetProcessDpiAwarenessContext(GUI_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE)) goto DpiAwarenessSet;
		}
	}

	// Fallback, and try to use SetProcessDpiAwareness (Windows 8.1)

	hMod_shcore = GetModuleHandle(L"SHCore.dll");
	if (hMod_shcore == NULL)
	{
		hMod_shcore = LoadLibrary(L"SHCore.dll");
	}

	if (hMod_shcore != NULL)
	{
		pSetProcessDpiAwareness = (SetProcessDpiAwareness_proc)GetProcAddress(hMod_shcore, "SetProcessDpiAwareness");
		if (pSetProcessDpiAwareness != NULL)
		{
			if (pSetProcessDpiAwareness(GUI_PROCESS_PER_MONITOR_DPI_AWARE) != E_INVALIDARG) goto DpiAwarenessSet;
			if (pSetProcessDpiAwareness(GUI_PROCESS_SYSTEM_DPI_AWARE) != E_INVALIDARG) goto DpiAwarenessSet;
		}
	}

	// Fallback, and try to use SetProcessDPIAware (Windows Vista)

	if (hMod_user32 != NULL)
	{
		pSetProcessDPIAware = (SetProcessDPIAware_proc)GetProcAddress(hMod_user32, "SetProcessDPIAware");
		if (pSetProcessDPIAware != NULL)
		{
			pSetProcessDPIAware();
		}
	}

DpiAwarenessSet:

	if (hMod_user32 != NULL)
	{
		pGetDpiForSystem = (GetDpiForSystem_proc)GetProcAddress(hMod_user32, "GetDpiForSystem");
		pGetDpiForWindow = (GetDpiForWindow_proc)GetProcAddress(hMod_user32, "GetDpiForWindow");
		pEnableNonClientDpiScaling = (EnableNonClientDpiScaling_proc)GetProcAddress(hMod_user32, "EnableNonClientDpiScaling");
		pAdjustWindowRectExForDpi = (AdjustWindowRectExForDpi_proc)GetProcAddress(hMod_user32, "AdjustWindowRectExForDpi");
	}

	// Init common controls.

	ICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ICC.dwICC = ICC_STANDARD_CLASSES;

	if (!InitCommonControlsEx(&ICC))
	{
		if (flags & WINGUI_FLAGS_REPORT_ERRORS)
			MessageBox(NULL, L"Failed to register the common control classes.",
				L"Error!", MB_OK);
	}

	mutexLock = CreateMutex(NULL, FALSE, NULL);
	if (mutexLock == 0)
	{
		if (flags & WINGUI_FLAGS_REPORT_ERRORS)
			MessageBox(NULL, L"Failed to create the mutex lock for WinGui classes.",
				L"Error!", MB_OK);

		// This is a fatal error for WinGui!

		return false;
	}

	return true;
}

bool WINAPI WinGui_SetAnchors(HWND hWnd, unsigned long anchors)
{
	DPI_VIRTUAL_INFO* virDpi;

	virDpi = (DPI_VIRTUAL_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virDpi)
	{
		virDpi->anchors = anchors;
		return true;
	}

	return false;
}

void dpi_ResizeClass(HWND hWnd, int monitorDpi, bool dpiChanged, RECT* pParentScale, const WCHAR* className)
{
	HWND control_hWnd;
	DPI_VIRTUAL_INFO* virDpi;
	int x, y, width, height;

	control_hWnd = nullptr;

	do
	{
		control_hWnd = FindWindowEx(hWnd, control_hWnd, className, nullptr);

		virDpi = (DPI_VIRTUAL_INFO*)GetWindowLongPtr(control_hWnd, GWLP_USERDATA);
		if (virDpi != nullptr)
		{
			if (dpiChanged)
				dpi_ResizeFont(control_hWnd, monitorDpi, virDpi);

			if (virDpi->anchors & ANCHOR_LEFT)
				x = (int)dpi_MulDiv(virDpi->x, monitorDpi, GLOBAL_APPLICATION_DPI);
			else
				x = ((pParentScale->right - pParentScale->left) - (int)dpi_MulDiv(virDpi->xOffset, monitorDpi, GLOBAL_APPLICATION_DPI));

			if (virDpi->anchors & ANCHOR_TOP)
				y = (int)dpi_MulDiv(virDpi->y, monitorDpi, GLOBAL_APPLICATION_DPI);
			else
				y = ((pParentScale->bottom - pParentScale->top) - (int)dpi_MulDiv(virDpi->yOffset, monitorDpi, GLOBAL_APPLICATION_DPI));

			if (!(virDpi->anchors & ANCHOR_RIGHT))
				width = (int)dpi_MulDiv(virDpi->width, monitorDpi, GLOBAL_APPLICATION_DPI);
			else
				width = ((pParentScale->right - pParentScale->left) - x) - (int)dpi_MulDiv(virDpi->wOffset, monitorDpi, GLOBAL_APPLICATION_DPI);

			if (!(virDpi->anchors & ANCHOR_BOTTOM))
				height = (int)dpi_MulDiv(virDpi->height, monitorDpi, GLOBAL_APPLICATION_DPI);
			else
				height = ((pParentScale->bottom - pParentScale->top) - y) - (int)dpi_MulDiv(virDpi->hOffset, monitorDpi, GLOBAL_APPLICATION_DPI);

			SetWindowPos(
				control_hWnd,
				nullptr,
				x,
				y,
				width,
				height,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

	} while (control_hWnd != NULL);
}

UINT dpi_HandleDpiChange(HWND hWnd, int monitorDpi, bool dpiChanged, RECT* pNewScale)
{
	RECT localScale;

	if (pNewScale == 0)
	{
		GetWindowRect(hWnd, &localScale);
		pNewScale = &localScale;
	}
	else
	{
		if (dpiChanged)
		{
			SetWindowPos(hWnd, nullptr, pNewScale->left, pNewScale->top,
				pNewScale->right - pNewScale->left, pNewScale->bottom - pNewScale->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"STATIC");
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"BUTTON");
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"LISTBOX");
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"EDIT");
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"COMBOBOX");
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, WC_LISTVIEW);
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, PROGRESS_CLASS);
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, DATETIMEPICK_CLASS);
	dpi_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, TRACKBAR_CLASS);

	return 0;
}

LRESULT CALLBACK winGui_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT WinArea;
	int monitorDpi;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;

	pWinGui_Class = 0;
	WndProc = 0;

	GetClassName(hWnd, className, 512);
	pWinGui_Class = winGui_FindClass(className);
	if (pWinGui_Class)
	{
		WndProc = pWinGui_Class->WndProc;
	}

	switch (uMsg)
	{
	case WM_NCCREATE:
		if (pEnableNonClientDpiScaling)
			pEnableNonClientDpiScaling(hWnd);

		break;
	case WM_DPICHANGED:
		return dpi_HandleDpiChange(hWnd, HIWORD(wParam), true, (RECT*)lParam);
	case WM_SIZE:
		GetWindowRect(hWnd, &WinArea);
		monitorDpi = GetDpiForWindow(hWnd);
		dpi_HandleDpiChange(hWnd, monitorDpi, false, &WinArea);

		break;
	default:
		break;
	}

	if (WndProc)
		return WndProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK winGui_ChildWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// TODO... Handle additional features and cleanup

	return 0;
}

ATOM
WINAPI
WinGui_RegisterClassEx(
	_In_ WNDCLASSEX* lpWndClass)
{
	WCHAR* pClassName;
	size_t length;
	WINGUI_CLASS_INFO* pWinGui_Class;

	length = sizeof(WCHAR) + (sizeof(WCHAR) * wcslen(lpWndClass->lpszClassName));
	pClassName = (WCHAR*)malloc(length);
	if (pClassName == 0) return 0;

	pWinGui_Class = (WINGUI_CLASS_INFO*)malloc(sizeof(WINGUI_CLASS_INFO));
	if (pWinGui_Class == 0)
	{
		free(pClassName);
		return 0;
	}

	wcscpy_s(pClassName, length / sizeof(WCHAR), lpWndClass->lpszClassName);

	pWinGui_Class->className = pClassName;
	pWinGui_Class->WndProc = lpWndClass->lpfnWndProc;
	pWinGui_Class->next = 0;

	winGui_AddClass(pWinGui_Class);

	lpWndClass->lpfnWndProc = winGui_WndProc;

	return RegisterClassEx(lpWndClass);
}

BOOL WinGui_UnregisterClass(
	_In_ const WCHAR* lpClassName,
	_In_opt_ HINSTANCE hInstance)
{
	WINGUI_CLASS_INFO* pWinGui_Class;

	pWinGui_Class = winGui_FindClass(lpClassName);
	if (pWinGui_Class)
	{
		winGui_RemoveClass(lpClassName);
		free(pWinGui_Class->className);
		free(pWinGui_Class);
	}

	return UnregisterClass(lpClassName, hInstance);
}

HWND WINAPI WinGui_CreateWindow(
	_In_ DWORD dwExStyle,
	_In_opt_ const WCHAR* lpClassName,
	_In_opt_ const WCHAR* lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_ bool bClientSize,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam,
	_In_opt_ LOGFONT* lpFont)
{
	HWND hWnd;
	DPI_VIRTUAL_INFO* virDpi;
	int x, y, width, height;
	int monitorDpi;
	RECT parentOrigin;

	hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
		X, Y, nWidth, nHeight,
		hWndParent, hMenu, hInstance, lpParam);

	if (hWnd)
	{
		if (!(dwStyle & WS_CHILD))
		{
			if (!bClientSize)
			{
				monitorDpi = GetDpiForWindow(hWnd);
				width = (int)dpi_MulDiv(nWidth, monitorDpi, GLOBAL_APPLICATION_DPI);
				height = (int)dpi_MulDiv(nHeight, monitorDpi, GLOBAL_APPLICATION_DPI);
				SetWindowPos(hWnd, NULL, 0, 0,
					width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				RECT WinArea;
				RECT ClientArea;

				monitorDpi = GetDpiForWindow(hWnd);

				GetWindowRect(hWnd, &WinArea);
				GetClientRect(hWnd, &ClientArea);

				int xExtra = WinArea.right - WinArea.left - ClientArea.right;
				int yExtra = WinArea.bottom - WinArea.top - ClientArea.bottom;

				ClientArea.right = ClientArea.left + (LONG)dpi_MulDiv(nWidth, monitorDpi, GLOBAL_APPLICATION_DPI);
				ClientArea.bottom = ClientArea.top + (LONG)dpi_MulDiv(nHeight, monitorDpi, GLOBAL_APPLICATION_DPI);

				int NewWidth = ClientArea.right - ClientArea.left;
				int NewHeight = ClientArea.bottom - ClientArea.top;

				SetWindowPos(hWnd, NULL, WinArea.right, WinArea.top,
					NewWidth + xExtra, NewHeight + yExtra, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		else
		{
			virDpi = (DPI_VIRTUAL_INFO*)malloc(sizeof(DPI_VIRTUAL_INFO));
			if (virDpi != 0)
			{
				memset(virDpi, 0, sizeof(DPI_VIRTUAL_INFO));

				WNDCLASSEX wcex;
				if (lpClassName != 0)
				{
					GetClassInfoEx(winGui_hInstance, lpClassName, &wcex);
					if (wcex.lpfnWndProc != winGui_ChildWndProc)
					{
						//virDpi->originalWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)winGui_WndProc);
						//SetClassLongPtr(hWnd, GCLP_WNDPROC, (LONG_PTR)winGui_ChildWndProc);
					}
				}

				virDpi->x = X;
				virDpi->y = Y;
				virDpi->width = nWidth;
				virDpi->height = nHeight;

				virDpi->lpFont = (LOGFONT*)malloc(sizeof(LOGFONT));
				if (virDpi->lpFont)
				{
					if (lpFont)
						memcpy(virDpi->lpFont, lpFont, sizeof(LOGFONT));
					else
					{
						memset(virDpi->lpFont, 0, sizeof(LOGFONT));
						monitorDpi = GetDpiForWindow(hWnd);
						winGui_GetdefaultFont(virDpi->lpFont, GLOBAL_APPLICATION_DPI);
						dpi_ResizeFont(hWnd, monitorDpi, virDpi);
					}
				}

				virDpi->anchors = ANCHOR_TOP | ANCHOR_LEFT;

				if (hWndParent)
				{
					monitorDpi = GetDpiForWindow(hWnd);
					GetWindowRect(hWndParent, &parentOrigin);

					x = (int)dpi_MulDiv(X, monitorDpi, GLOBAL_APPLICATION_DPI);
					y = (int)dpi_MulDiv(Y, monitorDpi, GLOBAL_APPLICATION_DPI);
					width = (int)dpi_MulDiv(nWidth, monitorDpi, GLOBAL_APPLICATION_DPI);
					height = (int)dpi_MulDiv(nHeight, monitorDpi, GLOBAL_APPLICATION_DPI);

					virDpi->xOffset = (long)dpi_PixelsToDips(
						(double)((double)parentOrigin.right - parentOrigin.left) - x, monitorDpi);
					virDpi->yOffset = (long)dpi_PixelsToDips(
						(double)((double)parentOrigin.bottom - parentOrigin.top) - y, monitorDpi);
					virDpi->wOffset = (long)dpi_PixelsToDips(
						(double)((double)parentOrigin.right - parentOrigin.left) - ((double)x + width), monitorDpi);
					virDpi->hOffset = (long)dpi_PixelsToDips(
						(double)((double)parentOrigin.bottom - parentOrigin.top) - ((double)y + height), monitorDpi);

					SetWindowPos(
						hWnd,
						nullptr,
						x,
						y,
						width,
						height,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}

				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)virDpi);
			}
		}
	}

	return hWnd;
}
