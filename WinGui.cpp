
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
	return pixels / (monitorDpi / WINGUI_APPLICATION_DPI);
}

void dpi_ResizeFont(HWND hWnd, int monitorDpi, DPI_VIRTUAL_INFO* virDpi)
{
	HFONT hFontOld;
	HFONT hFontNew;
	LOGFONT font;

	hFontOld = GetWindowFont(hWnd);

	if (virDpi->lpFont != 0)
	{
		memcpy(&font, virDpi->lpFont, sizeof(LOGFONT));
		font.lfHeight = (LONG)-dpi_MulDiv(virDpi->lpFont->lfHeight, monitorDpi, WINGUI_APPLICATION_DPI);
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

//
// This function is an alternative &/or fallback for winGui_GetDpiForWindow.
// If the monitor has a different ratio than 16:9, then the lowest DPI between X and Y is used.
// Cannot be implemented correctly due to how windows handles DPI changes.
// Bugs:
//       MonitorFromWindow does not always grab the new monitor during the WM_DPICHANGED event.
//       The non-client area DPI updates at a different time than the client area when called in the WM_DPICHANGED event.
//       DPI reported will be different from what is reported by windows.
//
// Only to be used when EnableNonClientDpiScaling is not supported. (Windows 10, version 1607)
//

UINT winGui_GetDpiForWindow_Fallback(HWND hWnd)
{
	HMONITOR hMonitor;
	MONITORINFO mInfo;
	double x_dpi, y_dpi;
	UINT monitorDpi;

	monitorDpi = 0;

	hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		memset(&mInfo, 0, sizeof(MONITORINFO));
		mInfo.cbSize = sizeof(MONITORINFO);

		if (GetMonitorInfo(hMonitor, &mInfo))
		{
			x_dpi = (((double)mInfo.rcMonitor.right - mInfo.rcMonitor.left) * (double)25.4) / (double)508;
			y_dpi = (((double)mInfo.rcMonitor.bottom - mInfo.rcMonitor.top) * (double)25.4) / (double)285.75;

			monitorDpi = (x_dpi > y_dpi) ? (UINT)y_dpi : (UINT)x_dpi;
		}
	}

	return monitorDpi;
}

UINT winGui_GetDpiForWindow(HWND hWnd)
{
	UINT monitorDpi;

	monitorDpi = 0;

	if (pGetDpiForWindow)
		monitorDpi = pGetDpiForWindow(hWnd);
	else if (pGetDpiForSystem)
		monitorDpi = pGetDpiForSystem();
	else
	{
		// GetDpiForSystem is not supported so neither is EnableNonClientDpiScaling
		monitorDpi = winGui_GetDpiForWindow_Fallback(hWnd);
	}

	if (monitorDpi == 0)
	{
		// Everything failed. Just use the applications DPI value.
		monitorDpi = WINGUI_APPLICATION_DPI;
	}
	else if (monitorDpi < WINGUI_MINIMUM_MONITOR_DPI)
	{
		monitorDpi = WINGUI_MINIMUM_MONITOR_DPI;
	}

	return monitorDpi;
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
				x = (int)dpi_MulDiv(virDpi->x, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				x = ((pParentScale->right - pParentScale->left) - (int)dpi_MulDiv(virDpi->xOffset, monitorDpi, WINGUI_APPLICATION_DPI));

			if (virDpi->anchors & ANCHOR_TOP)
				y = (int)dpi_MulDiv(virDpi->y, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				y = ((pParentScale->bottom - pParentScale->top) - (int)dpi_MulDiv(virDpi->yOffset, monitorDpi, WINGUI_APPLICATION_DPI));

			if (!(virDpi->anchors & ANCHOR_RIGHT))
				width = (int)dpi_MulDiv(virDpi->width, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				width = ((pParentScale->right - pParentScale->left) - x) - (int)dpi_MulDiv(virDpi->wOffset, monitorDpi, WINGUI_APPLICATION_DPI);

			if (!(virDpi->anchors & ANCHOR_BOTTOM))
				height = (int)dpi_MulDiv(virDpi->height, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				height = ((pParentScale->bottom - pParentScale->top) - y) - (int)dpi_MulDiv(virDpi->hOffset, monitorDpi, WINGUI_APPLICATION_DPI);

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
	DPI_VIRTUAL_INFO* virDpi;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;

	pWinGui_Class = 0;
	WndProc = 0;

	virDpi = (DPI_VIRTUAL_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virDpi != 0)
	{
		pWinGui_Class = virDpi->pClass;
		WndProc = pWinGui_Class->WndProc;
	}
	else
	{
		GetClassName(hWnd, className, 512);
		pWinGui_Class = winGui_FindClass(className);
		if (pWinGui_Class)
		{
			WndProc = pWinGui_Class->WndProc;
		}
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
		monitorDpi = winGui_GetDpiForWindow(hWnd);
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

// TODO... programmer may want to start the app on the screen that has the mouse.

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

	virDpi = (DPI_VIRTUAL_INFO*)malloc(sizeof(DPI_VIRTUAL_INFO));
	if (virDpi == 0) return 0;

	memset(virDpi, 0, sizeof(DPI_VIRTUAL_INFO));

	hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
		X, Y, nWidth, nHeight,
		hWndParent, hMenu, hInstance, lpParam);

	if (hWnd == 0)
	{
		free(virDpi);
		return 0;
	}

	monitorDpi = winGui_GetDpiForWindow(hWnd);

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
	virDpi->monitorDpi = monitorDpi;
	virDpi->pClass = winGui_FindClass(lpClassName);

	virDpi->lpFont = (LOGFONT*)malloc(sizeof(LOGFONT));
	if (virDpi->lpFont)
	{
		if (lpFont)
			memcpy(virDpi->lpFont, lpFont, sizeof(LOGFONT));
		else
		{
			memset(virDpi->lpFont, 0, sizeof(LOGFONT));
			winGui_GetdefaultFont(virDpi->lpFont, WINGUI_APPLICATION_DPI);
			dpi_ResizeFont(hWnd, monitorDpi, virDpi);
		}
	}

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)virDpi);

	if (hWnd)
	{
		if (!(dwStyle & WS_CHILD))
		{
			if (!bClientSize)
			{
				if (!(dwStyle & WS_MAXIMIZE))
				{
					width = (int)dpi_MulDiv(nWidth, monitorDpi, WINGUI_APPLICATION_DPI);
					height = (int)dpi_MulDiv(nHeight, monitorDpi, WINGUI_APPLICATION_DPI);
					SetWindowPos(hWnd, NULL, 0, 0,
						width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
			else
			{
				if (!(dwStyle & WS_MAXIMIZE))
				{
					RECT WinArea;
					RECT ClientArea;

					GetWindowRect(hWnd, &WinArea);
					GetClientRect(hWnd, &ClientArea);

					int xExtra = WinArea.right - WinArea.left - ClientArea.right;
					int yExtra = WinArea.bottom - WinArea.top - ClientArea.bottom;

					ClientArea.right = ClientArea.left + (LONG)dpi_MulDiv(nWidth, monitorDpi, WINGUI_APPLICATION_DPI);
					ClientArea.bottom = ClientArea.top + (LONG)dpi_MulDiv(nHeight, monitorDpi, WINGUI_APPLICATION_DPI);

					int NewWidth = ClientArea.right - ClientArea.left;
					int NewHeight = ClientArea.bottom - ClientArea.top;

					SetWindowPos(hWnd, NULL, WinArea.right, WinArea.top,
						NewWidth + xExtra, NewHeight + yExtra, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
		}
		else
		{
			virDpi->anchors = ANCHOR_TOP | ANCHOR_LEFT;

			if (hWndParent)
			{
				GetWindowRect(hWndParent, &parentOrigin);

				x = (int)dpi_MulDiv(X, monitorDpi, WINGUI_APPLICATION_DPI);
				y = (int)dpi_MulDiv(Y, monitorDpi, WINGUI_APPLICATION_DPI);
				width = (int)dpi_MulDiv(nWidth, monitorDpi, WINGUI_APPLICATION_DPI);
				height = (int)dpi_MulDiv(nHeight, monitorDpi, WINGUI_APPLICATION_DPI);

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
		}
	}

	return hWnd;
}

bool WinGui_SetWindowUserData(HWND hWnd, void* UserData)
{
	DPI_VIRTUAL_INFO* virDpi;

	virDpi = (DPI_VIRTUAL_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virDpi)
	{
		virDpi->userData = UserData;
		return true;
	}

	return false;
}

void* WinGui_GetWindowUserData(HWND hWnd)
{
	DPI_VIRTUAL_INFO* virDpi;

	virDpi = (DPI_VIRTUAL_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virDpi)
	{
		return virDpi->userData;
	}

	return 0;
}
