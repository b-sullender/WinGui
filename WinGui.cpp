
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

void dpi_ResizeFont(HWND hWnd, int monitorDpi, WIN_VIR_INFO* virWin)
{
	HFONT hFontOld;
	HFONT hFontNew;
	LOGFONT font;

	hFontOld = GetWindowFont(hWnd);

	if (virWin->lpFont != 0)
	{
		memcpy(&font, virWin->lpFont, sizeof(LOGFONT));
		font.lfHeight = (LONG)-dpi_MulDiv(virWin->lpFont->lfHeight, monitorDpi, WINGUI_APPLICATION_DPI);
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
	else if (monitorDpi < WINGUI_APPLICATION_DPI)
	{
		monitorDpi = WINGUI_APPLICATION_DPI;
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
	WIN_VIR_INFO* virWin;

	virWin = (WIN_VIR_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virWin)
	{
		virWin->anchors = anchors;
		return true;
	}

	return false;
}

void winGui_UpdateHeaderForDpiChange(HWND hWnd, WIN_VIR_INFO* virWin, int old_monitorDpi, int monitorDpi)
{
	HWND hWndHdr;
	long nColumns;
	HDITEM hdi;

	hWndHdr = FindWindowEx(hWnd, nullptr, WC_HEADER, nullptr);
	if (hWndHdr == 0) return;
	nColumns = (int)SendMessage(hWndHdr, HDM_GETITEMCOUNT, 0, 0);

	memset(&hdi, 0, sizeof(HDITEM));

	for (int i = 0; i < nColumns; i++)
	{
		hdi.mask = HDI_WIDTH;
		Header_GetItem(hWndHdr, i, &hdi);
		hdi.cxy = (int)dpi_MulDiv(hdi.cxy, monitorDpi, old_monitorDpi);
		Header_SetItem(hWndHdr, i, &hdi);
	}
}

void winGui_ResizeClass(HWND hWnd, int monitorDpi, bool dpiChanged, RECT* pParentScale, const WCHAR* className)
{
	HWND control_hWnd;
	WIN_VIR_INFO* virWin;
	int old_monitorDpi, x, y, width, height;

	control_hWnd = nullptr;

	do
	{
		control_hWnd = FindWindowEx(hWnd, control_hWnd, className, nullptr);

		virWin = (WIN_VIR_INFO*)GetWindowLongPtr(control_hWnd, GWLP_USERDATA);
		if (virWin != nullptr)
		{
			if (dpiChanged)
				dpi_ResizeFont(control_hWnd, monitorDpi, virWin);

			if (virWin->anchors & ANCHOR_LEFT)
				x = (int)dpi_MulDiv(virWin->x, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				x = ((pParentScale->right - pParentScale->left) - (int)dpi_MulDiv(virWin->xOffset, monitorDpi, WINGUI_APPLICATION_DPI));

			if (virWin->anchors & ANCHOR_TOP)
				y = (int)dpi_MulDiv(virWin->y, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				y = ((pParentScale->bottom - pParentScale->top) - (int)dpi_MulDiv(virWin->yOffset, monitorDpi, WINGUI_APPLICATION_DPI));

			if (!(virWin->anchors & ANCHOR_RIGHT))
				width = (int)dpi_MulDiv(virWin->width, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				width = ((pParentScale->right - pParentScale->left) - x) - (int)dpi_MulDiv(virWin->wOffset, monitorDpi, WINGUI_APPLICATION_DPI);

			if (!(virWin->anchors & ANCHOR_BOTTOM))
				height = (int)dpi_MulDiv(virWin->height, monitorDpi, WINGUI_APPLICATION_DPI);
			else
				height = ((pParentScale->bottom - pParentScale->top) - y) - (int)dpi_MulDiv(virWin->hOffset, monitorDpi, WINGUI_APPLICATION_DPI);

			old_monitorDpi = virWin->monitorDpi;
			virWin->monitorDpi = monitorDpi;

			if (dpiChanged)
				winGui_UpdateHeaderForDpiChange(control_hWnd, virWin, old_monitorDpi, monitorDpi);

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

UINT winGui_WindowResizeHandler(HWND hWnd, WIN_VIR_INFO* virWin, int monitorDpi, bool dpiChanged, RECT* pNewScale)
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
			virWin->handleResize = false;

			SetWindowPos(hWnd, nullptr, pNewScale->left, pNewScale->top,
				pNewScale->right - pNewScale->left, pNewScale->bottom - pNewScale->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			virWin->handleResize = true;
		}
	}

	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"STATIC");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"BUTTON");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"LISTBOX");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"EDIT");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"COMBOBOX");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"SCROLLBAR");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, L"RICHEDIT_CLASS");
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, WC_LISTVIEW);
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, PROGRESS_CLASS);
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, DATETIMEPICK_CLASS);
	winGui_ResizeClass(hWnd, monitorDpi, dpiChanged, pNewScale, TRACKBAR_CLASS);

	return 0;
}

LRESULT CALLBACK winGui_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT WinArea;
	int monitorDpi;
	WIN_VIR_INFO* virWin;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;

	pWinGui_Class = 0;
	WndProc = 0;

	virWin = (WIN_VIR_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virWin != 0)
	{
		pWinGui_Class = virWin->pClass;
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
		if (virWin)
		{
			return winGui_WindowResizeHandler(hWnd, virWin, HIWORD(wParam), true, (RECT*)lParam);
		}

	case WM_SIZE:
		if (virWin)
		{
			if (virWin->handleResize)
			{
				GetWindowRect(hWnd, &WinArea);
				monitorDpi = winGui_GetDpiForWindow(hWnd);
				winGui_WindowResizeHandler(hWnd, virWin, monitorDpi, false, &WinArea);
			}
		}

		break;
	case WM_DESTROY:
		if (virWin != 0)
		{
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)0);

			if (virWin->lpFont != 0)
				free(virWin->lpFont);

			free(virWin);
		}

		break;
	case WM_NOTIFY:
	{
		break;
	}
	default:
		break;
	}

	if (WndProc)
		return WndProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK winGui_Control_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WIN_VIR_INFO* virWin;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;

	pWinGui_Class = 0;
	WndProc = 0;

	virWin = (WIN_VIR_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virWin)
	{
		pWinGui_Class = virWin->pClass;
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
	case WM_DESTROY:
		if (virWin)
		{
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)0);

			if (virWin->lpFont != 0)
				free(virWin->lpFont);

			free(virWin);
		}

		break;
	default:
		break;
	}

	if (WndProc)
		return WndProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WINGUI_CLASS_INFO* winGui_RegisterInternalControl(
	_In_ HWND ClassGateWay,
	_In_ const WCHAR* lpClassName)
{
	WCHAR* pClassName;
	size_t length;
	WINGUI_CLASS_INFO* pWinGui_Class;

	length = sizeof(WCHAR) + (sizeof(WCHAR) * wcslen(lpClassName));
	pClassName = (WCHAR*)malloc(length);
	if (pClassName == 0) return 0;

	pWinGui_Class = (WINGUI_CLASS_INFO*)malloc(sizeof(WINGUI_CLASS_INFO));
	if (pWinGui_Class == 0)
	{
		free(pClassName);
		return 0;
	}

	wcscpy_s(pClassName, length / sizeof(WCHAR), lpClassName);

	pWinGui_Class->className = pClassName;
	pWinGui_Class->WndProc = (WNDPROC)GetWindowLongPtr(ClassGateWay, GWLP_WNDPROC);
	pWinGui_Class->next = 0;

	winGui_AddClass(pWinGui_Class);

	return pWinGui_Class;
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
	WIN_VIR_INFO* virWin;
	int x, y, width, height;
	int monitorDpi;
	RECT parentOrigin;

	if (lpClassName == 0)
		return 0;

	virWin = (WIN_VIR_INFO*)malloc(sizeof(WIN_VIR_INFO));
	if (virWin == 0) return 0;

	memset(virWin, 0, sizeof(WIN_VIR_INFO));

	hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
		X, Y, nWidth, nHeight,
		hWndParent, hMenu, hInstance, lpParam);

	if (hWnd == 0)
	{
		free(virWin);
		return 0;
	}

	monitorDpi = winGui_GetDpiForWindow(hWnd);

	virWin->x = X;
	virWin->y = Y;
	virWin->width = nWidth;
	virWin->height = nHeight;
	virWin->monitorDpi = monitorDpi;
	virWin->pClass = winGui_FindClass(lpClassName);

	virWin->lpFont = (LOGFONT*)malloc(sizeof(LOGFONT));
	if (virWin->lpFont)
	{
		if (lpFont)
			memcpy(virWin->lpFont, lpFont, sizeof(LOGFONT));
		else
		{
			memset(virWin->lpFont, 0, sizeof(LOGFONT));
			winGui_GetdefaultFont(virWin->lpFont, WINGUI_APPLICATION_DPI);
			dpi_ResizeFont(hWnd, monitorDpi, virWin);
		}
	}

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)virWin);

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

		virWin->handleResize = true;
	}
	else
	{
		WINGUI_CLASS_INFO* pWinGui_Class;

		pWinGui_Class = winGui_FindClass(lpClassName);
		if (pWinGui_Class)
		{
			virWin->pClass = pWinGui_Class;
		}
		else
		{
			pWinGui_Class = winGui_RegisterInternalControl(hWnd, lpClassName);
			virWin->pClass = pWinGui_Class;
		}

		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)winGui_Control_WndProc);

		virWin->anchors = ANCHOR_TOP | ANCHOR_LEFT;

		if (hWndParent)
		{
			GetWindowRect(hWndParent, &parentOrigin);

			x = (int)dpi_MulDiv(X, monitorDpi, WINGUI_APPLICATION_DPI);
			y = (int)dpi_MulDiv(Y, monitorDpi, WINGUI_APPLICATION_DPI);
			width = (int)dpi_MulDiv(nWidth, monitorDpi, WINGUI_APPLICATION_DPI);
			height = (int)dpi_MulDiv(nHeight, monitorDpi, WINGUI_APPLICATION_DPI);

			virWin->xOffset = (long)dpi_PixelsToDips(
				(double)((double)parentOrigin.right - parentOrigin.left) - x, monitorDpi);
			virWin->yOffset = (long)dpi_PixelsToDips(
				(double)((double)parentOrigin.bottom - parentOrigin.top) - y, monitorDpi);
			virWin->wOffset = (long)dpi_PixelsToDips(
				(double)((double)parentOrigin.right - parentOrigin.left) - ((double)x + width), monitorDpi);
			virWin->hOffset = (long)dpi_PixelsToDips(
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

	return hWnd;
}

bool WinGui_SetWindowUserData(HWND hWnd, void* UserData)
{
	WIN_VIR_INFO* virWin;

	virWin = (WIN_VIR_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virWin)
	{
		virWin->userData = UserData;
		return true;
	}

	return false;
}

void* WinGui_GetWindowUserData(HWND hWnd)
{
	WIN_VIR_INFO* virWin;

	virWin = (WIN_VIR_INFO*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (virWin)
	{
		return virWin->userData;
	}

	return 0;
}
