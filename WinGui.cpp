
#include "WinGui.h"

// Common controls dependencies.

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// hInstance

static HINSTANCE winGui_hInstance;

// DPI function pointers.

static GetDpiForMonitor_proc pGetDpiForMonitor;
static GetDpiForSystem_proc pGetDpiForSystem;
static GetDpiForWindow_proc pGetDpiForWindow;
static SetProcessDPIAware_proc pSetProcessDPIAware;
static SetProcessDpiAwareness_proc pSetProcessDpiAwareness;
static SetProcessDpiAwarenessContext_proc pSetProcessDpiAwarenessContext;
static EnableNonClientDpiScaling_proc pEnableNonClientDpiScaling;
static AdjustWindowRectExForDpi_proc pAdjustWindowRectExForDpi;

static HANDLE classMutexLock = 0;
static WINGUI_CLASS_INFO* pWinGui_FirstClass = 0;

static HANDLE winMutexLock = 0;
static WINGUI_WINDOW_INFO* pWinGui_FirstWindow = 0;

double dpi_MulDiv(double nNumber, double nNumerator, double nDenominator)
{
	return (nNumber * nNumerator) / nDenominator;
}

double dpi_PixelsToDips(double pixels, double monitorDpi)
{
	return pixels / (monitorDpi / WINGUI_APPLICATION_DPI);
}

void dpi_ResizeFont(HWND hWnd, int monitorDpi, WINGUI_WINDOW_INFO* virWin)
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
	WaitForSingleObject(classMutexLock, INFINITE);

	if (pWinGui_FirstClass == 0)
		pWinGui_FirstClass = pWinGui_Class;
	else
	{
		pWinGui_Class->next = pWinGui_FirstClass;
		pWinGui_FirstClass = pWinGui_Class;
	}

	ReleaseMutex(classMutexLock);
}

bool winGui_RemoveClass(const WCHAR* pClassName)
{
	bool result;
	WINGUI_CLASS_INFO* pThisClass, * pPrevClass;

	result = false;
	pPrevClass = 0;

	WaitForSingleObject(classMutexLock, INFINITE);

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
	ReleaseMutex(classMutexLock);

	return result;
}

WINGUI_CLASS_INFO* winGui_FindClass(const WCHAR* pClassName)
{
	WINGUI_CLASS_INFO* pThisClass, * result;

	WaitForSingleObject(classMutexLock, INFINITE);

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

	ReleaseMutex(classMutexLock);

	return result;
}

void winGui_AddWindow(WINGUI_WINDOW_INFO* pWinGui_Window)
{
	WaitForSingleObject(winMutexLock, INFINITE);

	if (pWinGui_FirstWindow == 0)
		pWinGui_FirstWindow = pWinGui_Window;
	else
	{
		pWinGui_Window->next = pWinGui_FirstWindow;
		pWinGui_FirstWindow = pWinGui_Window;
	}

	ReleaseMutex(winMutexLock);
}

bool winGui_RemoveWindow(HWND hWnd)
{
	bool result;
	WINGUI_WINDOW_INFO* pThisWindow, * pPrevWindow;

	result = false;
	pPrevWindow = 0;

	WaitForSingleObject(winMutexLock, INFINITE);

	if (pWinGui_FirstWindow != 0)
	{
		if (hWnd == pWinGui_FirstWindow->hWnd)
		{
			pWinGui_FirstWindow = pWinGui_FirstWindow->next;
			goto removeClass_exit;
		}

		pPrevWindow = pWinGui_FirstWindow;
		pThisWindow = pWinGui_FirstWindow->next;
		while (pThisWindow != 0)
		{
			if (hWnd == pThisWindow->hWnd)
			{
				if (pPrevWindow)
					pPrevWindow->next = pThisWindow->next;

				break;
			}
			pPrevWindow = pThisWindow;
			pThisWindow = pThisWindow->next;
		}
	}

removeClass_exit:
	ReleaseMutex(winMutexLock);

	return result;
}

WINGUI_WINDOW_INFO* winGui_FindWindow(HWND hWnd)
{
	WINGUI_WINDOW_INFO* pThisWindow, * result;

	WaitForSingleObject(winMutexLock, INFINITE);

	result = 0;

	pThisWindow = pWinGui_FirstWindow;
	while (pThisWindow != 0)
	{
		if (hWnd == pThisWindow->hWnd)
		{
			result = pThisWindow;
			break;
		}
		pThisWindow = pThisWindow->next;
	}

	ReleaseMutex(winMutexLock);

	return result;
}

//
// winGui_GetDpiForMonitor Function
// 
// When GetDpiForMonitor is unsupported, this function behaves in the following manner.
// If the monitor has a different ratio than 16:9, then the lowest DPI between X and Y is used.
// Note, attempting to implement custom WM_DPICHANGED detection using WM_MOVE -
//       cannot be implemented correctly due to how windows handles DPI changes.
// Issue Notes:
//       MonitorFromWindow does not always grab the new monitor during the WM_MOVE event.
//       The non-client area DPI updates at a different time than the client area when called in the WM_DPICHANGED event.
//       DPI reported will be different from what is reported by windows.
//

UINT winGui_GetDpiForMonitor(HMONITOR hMonitor)
{
	MONITORINFO mInfo;
	UINT x_dpi, y_dpi;
	double dxDpi, dyDpi;
	UINT monitorDpi;

	x_dpi = y_dpi = 0;
	monitorDpi = 0;

	if (pGetDpiForMonitor)
	{
		if (pGetDpiForMonitor(hMonitor, GUI_MDT_RAW_DPI, &x_dpi, &y_dpi) == S_OK)
		{
			monitorDpi = y_dpi;
		}
	}

	if (monitorDpi == 0)
	{
		memset(&mInfo, 0, sizeof(MONITORINFO));
		mInfo.cbSize = sizeof(MONITORINFO);

		if (GetMonitorInfo(hMonitor, &mInfo))
		{
			dxDpi = (((double)mInfo.rcMonitor.right - mInfo.rcMonitor.left) * (double)25.4) / (double)508;
			dyDpi = (((double)mInfo.rcMonitor.bottom - mInfo.rcMonitor.top) * (double)25.4) / (double)285.75;

			monitorDpi = (dxDpi > dyDpi) ? (UINT)dyDpi : (UINT)dxDpi;
		}
	}

	return monitorDpi;
}

UINT winGui_GetDpiForWindow(HWND hWnd)
{
	HMONITOR hMonitor;
	UINT monitorDpi;

	monitorDpi = 0;

	if (pGetDpiForWindow)
		monitorDpi = pGetDpiForWindow(hWnd);
	else
	{
		hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		if (hMonitor)
		{
			monitorDpi = winGui_GetDpiForMonitor(hMonitor);
		}
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

bool winGui_MoveWindowToMonitor(HWND hWnd, HMONITOR hMonitor)
{
	HMONITOR hOldMonitor;
	MONITORINFOEX desInfoEx, srcInfoEx;
	int xLoc, yLoc;
	UINT old_monitorDpi, monitorDpi;
	RECT winArea;

	memset(&desInfoEx, 0, sizeof(MONITORINFOEX));
	memset(&srcInfoEx, 0, sizeof(MONITORINFOEX));

	desInfoEx.cbSize = sizeof(MONITORINFOEX);
	srcInfoEx.cbSize = sizeof(MONITORINFOEX);

	if (!GetMonitorInfo(hMonitor, &desInfoEx))
		return false;

	hOldMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (!GetMonitorInfo(hOldMonitor, &srcInfoEx))
		return false;

	if (!GetWindowRect(hWnd, &winArea))
		return false;

	if ((desInfoEx.rcMonitor.left == srcInfoEx.rcMonitor.left) &&
		(desInfoEx.rcMonitor.right == srcInfoEx.rcMonitor.right) &&
		(desInfoEx.rcMonitor.top == srcInfoEx.rcMonitor.top) &&
		(desInfoEx.rcMonitor.bottom == srcInfoEx.rcMonitor.bottom))
	{
		return true; // Its the same monitor.
	}

	old_monitorDpi = winGui_GetDpiForMonitor(hOldMonitor);
	monitorDpi = winGui_GetDpiForMonitor(hMonitor);

	// Get offsets for the current monitor.

	xLoc = winArea.left - srcInfoEx.rcMonitor.left;
	yLoc = winArea.top - srcInfoEx.rcMonitor.top;

	// Adjust for the DPI of the new monitor.

	xLoc = (int)dpi_MulDiv(xLoc, monitorDpi, old_monitorDpi);
	yLoc = (int)dpi_MulDiv(yLoc, monitorDpi, old_monitorDpi);

	// Make relative to the new monitor & move the window.

	xLoc = desInfoEx.rcMonitor.left + xLoc;
	yLoc = desInfoEx.rcMonitor.top + yLoc;

	SetWindowPos(hWnd, nullptr, xLoc, yLoc, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	return false;
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
	pGetDpiForMonitor = NULL;

	// Try to use SetProcessDpiAwarenessContext (Windows 10)

	hMod_user32 = GetModuleHandle(L"user32.dll");
	if (hMod_user32 == NULL)
	{
		hMod_user32 = LoadLibrary(L"user32.dll");
	}

	hMod_shcore = GetModuleHandle(L"SHCore.dll");
	if (hMod_shcore == NULL)
	{
		hMod_shcore = LoadLibrary(L"SHCore.dll");
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

	if (hMod_shcore != NULL)
	{
		pGetDpiForMonitor = (GetDpiForMonitor_proc)GetProcAddress(hMod_shcore, "GetDpiForMonitor");
	}

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

	classMutexLock = CreateMutex(NULL, FALSE, NULL);
	if (classMutexLock == 0)
	{
		if (flags & WINGUI_FLAGS_REPORT_ERRORS)
			MessageBox(NULL, L"Failed to create the mutex lock for WinGui classes.",
				L"Error!", MB_OK);

		// This is a fatal error for WinGui!

		return false;
	}

	winMutexLock = CreateMutex(NULL, FALSE, NULL);
	if (winMutexLock == 0)
	{
		if (flags & WINGUI_FLAGS_REPORT_ERRORS)
			MessageBox(NULL, L"Failed to create the mutex lock for WinGui windows",
				L"Error!", MB_OK);
	}

	return true;
}

bool WINAPI WinGui_SetAnchors(HWND hWnd, ULONG anchors)
{
	WINGUI_WINDOW_INFO* virWin;
	WINDOWPLACEMENT winPos;
	int x, y, width, height;
	HWND hWndParent;
	RECT parentClientArea;

	virWin = winGui_FindWindow(hWnd);
	if (virWin)
	{
		virWin->anchors = anchors;
		hWndParent = GetParent(hWnd);
		if (hWndParent)
		{
			memset(&winPos, 0, sizeof(WINDOWPLACEMENT));
			winPos.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &winPos);

			GetClientRect(hWndParent, &parentClientArea);

			x = winPos.rcNormalPosition.left;
			y = winPos.rcNormalPosition.top;
			width = winPos.rcNormalPosition.right - winPos.rcNormalPosition.left;
			height = winPos.rcNormalPosition.bottom - winPos.rcNormalPosition.top;

			virWin->xOffset = (parentClientArea.right - parentClientArea.left) - x;
			virWin->yOffset = (parentClientArea.bottom - parentClientArea.top) - y;
			virWin->wOffset = (parentClientArea.right - parentClientArea.left) - (x + width);
			virWin->hOffset = (parentClientArea.bottom - parentClientArea.top) - (y + height);

			if (virWin->anchors & WINGUI_ANCHOR_LEFT)
				x = (int)virWin->x;
			else
				x = ((parentClientArea.right - parentClientArea.left) - (int)virWin->xOffset);

			if (virWin->anchors & WINGUI_ANCHOR_TOP)
				y = (int)virWin->y;
			else
				y = ((parentClientArea.bottom - parentClientArea.top) - (int)virWin->yOffset);

			if (!(virWin->anchors & WINGUI_ANCHOR_RIGHT))
				width = (int)virWin->width;
			else
				width = ((parentClientArea.right - parentClientArea.left) - x) - (int)virWin->wOffset;

			if (!(virWin->anchors & WINGUI_ANCHOR_BOTTOM))
				height = (int)virWin->height;
			else
				height = ((parentClientArea.bottom - parentClientArea.top) - y) - (int)virWin->hOffset;

			SetWindowPos(
				hWnd,
				nullptr,
				x,
				y,
				width,
				height,
				SWP_NOZORDER | SWP_NOACTIVATE);

			return true;
		}
	}

	return false;
}

void winGui_UpdateHeaderForDpiChange(HWND hWnd, WINGUI_WINDOW_INFO* virWin, int old_monitorDpi, int monitorDpi)
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

void winGui_ResizeMDIChildWindows(HWND hWnd, int monitorDpi)
{
	HWND child_hWnd;
	WINGUI_WINDOW_INFO* virWin;
	WINDOWPLACEMENT winPos;
	int old_monitorDpi;

	child_hWnd = nullptr;

	do
	{
		child_hWnd = FindWindowEx(hWnd, child_hWnd, nullptr, nullptr);

		virWin = winGui_FindWindow(child_hWnd);
		if (virWin)
		{
			old_monitorDpi = virWin->monitorDpi;
			virWin->monitorDpi = monitorDpi;

			memset(&winPos, 0, sizeof(WINDOWPLACEMENT));
			winPos.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(child_hWnd, &winPos);

			if (winPos.showCmd == SW_SHOWMAXIMIZED)
			{
				SendMessage(child_hWnd, WM_DPICHANGED, (WPARAM)((size_t)monitorDpi << 16), 0);
			}
			else
			{
				winPos.rcNormalPosition.left = (int)dpi_MulDiv(winPos.rcNormalPosition.left, monitorDpi, old_monitorDpi);
				winPos.rcNormalPosition.top = (int)dpi_MulDiv(winPos.rcNormalPosition.top, monitorDpi, old_monitorDpi);
				winPos.rcNormalPosition.right = (int)dpi_MulDiv(winPos.rcNormalPosition.right, monitorDpi, old_monitorDpi);
				winPos.rcNormalPosition.bottom = (int)dpi_MulDiv(winPos.rcNormalPosition.bottom, monitorDpi, old_monitorDpi);

				SendMessage(child_hWnd, WM_DPICHANGED, (WPARAM)((size_t)monitorDpi << 16), (LPARAM)&winPos.rcNormalPosition);
			}
		}

	} while (child_hWnd != NULL);
}

// Note, for MDI child windows, during a DPI change event the MDICLIENT control will resize its MDI child windows border.
//   This results in winGui_ResizeClasses being called first with "dpiChanged" set to false. 

void winGui_ResizeClasses(HWND hWnd, int monitorDpi, bool dpiChanged, RECT* pParentScale)
{
	HWND control_hWnd;
	WINGUI_WINDOW_INFO* virWin;
	int old_monitorDpi, x, y, width, height;
	RECT parentClientArea;

	control_hWnd = nullptr;

	do
	{
		control_hWnd = FindWindowEx(hWnd, control_hWnd, nullptr, nullptr);

		virWin = winGui_FindWindow(control_hWnd);
		if (virWin != nullptr)
		{
			if (dpiChanged)
			{
				dpi_ResizeFont(control_hWnd, monitorDpi, virWin);

				old_monitorDpi = virWin->monitorDpi;
				virWin->monitorDpi = monitorDpi;

				virWin->x = (int)dpi_MulDiv(virWin->x, monitorDpi, old_monitorDpi);
				virWin->y = (int)dpi_MulDiv(virWin->y, monitorDpi, old_monitorDpi);
				virWin->width = (int)dpi_MulDiv(virWin->width, monitorDpi, old_monitorDpi);
				virWin->height = (int)dpi_MulDiv(virWin->height, monitorDpi, old_monitorDpi);
				virWin->xOffset = (int)dpi_MulDiv(virWin->xOffset, monitorDpi, old_monitorDpi);
				virWin->yOffset = (int)dpi_MulDiv(virWin->yOffset, monitorDpi, old_monitorDpi);
				virWin->wOffset = (int)dpi_MulDiv(virWin->wOffset, monitorDpi, old_monitorDpi);
				virWin->hOffset = (int)dpi_MulDiv(virWin->hOffset, monitorDpi, old_monitorDpi);
			}

			GetClientRect(hWnd, &parentClientArea);

			if (virWin->anchors & WINGUI_ANCHOR_LEFT)
				x = (int)virWin->x;
			else
				x = ((parentClientArea.right - parentClientArea.left) - (int)virWin->xOffset);

			if (virWin->anchors & WINGUI_ANCHOR_TOP)
				y = (int)virWin->y;
			else
				y = ((parentClientArea.bottom - parentClientArea.top) - (int)virWin->yOffset);

			if (!(virWin->anchors & WINGUI_ANCHOR_RIGHT))
				width = (int)virWin->width;
			else
				width = ((parentClientArea.right - parentClientArea.left) - x) - (int)virWin->wOffset;

			if (!(virWin->anchors & WINGUI_ANCHOR_BOTTOM))
				height = (int)virWin->height;
			else
				height = ((parentClientArea.bottom - parentClientArea.top) - y) - (int)virWin->hOffset;

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

			if (dpiChanged)
			{
				if (wcscmp(L"MDICLIENT", virWin->pClass->className) == 0)
				{
					winGui_ResizeMDIChildWindows(control_hWnd, monitorDpi);
				}
			}
		}

	} while (control_hWnd != NULL);
}

UINT winGui_WindowResizeHandler(HWND hWnd, WINGUI_WINDOW_INFO* virWin, int monitorDpi, bool dpiChanged, RECT* pNewScale)
{
	RECT localScale;

	if (pNewScale == 0)
	{
		GetWindowRect(hWnd, &localScale);
		pNewScale = &localScale;

		SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
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

	winGui_ResizeClasses(hWnd, monitorDpi, dpiChanged, pNewScale);

	return 0;
}

LRESULT CALLBACK winGui_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT* lpCreateParams;
	MDICREATESTRUCT* lpMDICS;
	RECT WinArea;
	RECT ClientArea;
	int monitorDpi;
	WINGUI_WINDOW_INFO* virWin;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;
	int NewWidth, NewHeight;

	pWinGui_Class = 0;
	WndProc = 0;

	virWin = winGui_FindWindow(hWnd);
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

		if (pWinGui_Class)
		{
			if (pWinGui_Class->MDI_CHILD)
			{
				lpCreateParams = (CREATESTRUCT*)lParam;
				lpMDICS = (MDICREATESTRUCT*)lpCreateParams->lpCreateParams;
				virWin = (WINGUI_WINDOW_INFO*)lpMDICS->lParam;
			}
			else
			{
				lpCreateParams = (CREATESTRUCT*)lParam;
				virWin = (WINGUI_WINDOW_INFO*)lpCreateParams->lpCreateParams;
			}

			if (virWin)
			{
				virWin->hWnd = hWnd;
				virWin->monitorDpi = winGui_GetDpiForWindow(hWnd);
				winGui_AddWindow(virWin);
			}
		}

		break;
	case WM_DPICHANGED:
		if (virWin)
		{
			winGui_WindowResizeHandler(hWnd, virWin, HIWORD(wParam), true, (RECT*)lParam);
		}

		break;
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

			winGui_RemoveWindow(hWnd);

			if (virWin->lpFont != 0)
				free(virWin->lpFont);

			free(virWin);
		}

		break;
	case WM_CREATE:
		if (virWin)
		{
			lpCreateParams = (CREATESTRUCT*)lParam;

			dpi_ResizeFont(hWnd, virWin->monitorDpi, virWin);

			if (!virWin->bClientSize)
			{
				NewWidth = (int)dpi_MulDiv(virWin->width, virWin->monitorDpi, WINGUI_APPLICATION_DPI);
				NewHeight = (int)dpi_MulDiv(virWin->height, virWin->monitorDpi, WINGUI_APPLICATION_DPI);
				SetWindowPos(hWnd, NULL, 0, 0,
					NewWidth, NewHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				GetWindowRect(hWnd, &WinArea);
				GetClientRect(hWnd, &ClientArea);

				int xExtra = WinArea.right - WinArea.left - ClientArea.right;
				int yExtra = WinArea.bottom - WinArea.top - ClientArea.bottom;

				ClientArea.right = ClientArea.left + (LONG)dpi_MulDiv(virWin->width, virWin->monitorDpi, WINGUI_APPLICATION_DPI);
				ClientArea.bottom = ClientArea.top + (LONG)dpi_MulDiv(virWin->height, virWin->monitorDpi, WINGUI_APPLICATION_DPI);

				NewWidth = ClientArea.right - ClientArea.left;
				NewHeight = ClientArea.bottom - ClientArea.top;

				SetWindowPos(hWnd, NULL, WinArea.right, WinArea.top,
					NewWidth + xExtra, NewHeight + yExtra, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}

			POINT xy;
			HMONITOR hMonitor;

			if (virWin->iMonitor & WINGUI_MONITOR_DEFAULT)
			{
				xy.x = 0;
				xy.y = 0;
				hMonitor = MonitorFromPoint(xy, MONITOR_DEFAULTTOPRIMARY);
				winGui_MoveWindowToMonitor(hWnd, hMonitor);
			}
			else if (virWin->iMonitor & WINGUI_MONITOR_FROM_MOUSE)
			{
				GetCursorPos(&xy);
				hMonitor = MonitorFromPoint(xy, MONITOR_DEFAULTTONEAREST);
				winGui_MoveWindowToMonitor(hWnd, hMonitor);
			}
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

LRESULT CALLBACK winGui_Control_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WINGUI_WINDOW_INFO* virWin;
	WCHAR className[512];
	WINGUI_CLASS_INFO* pWinGui_Class;
	WNDPROC WndProc;

	pWinGui_Class = 0;
	WndProc = 0;

	virWin = winGui_FindWindow(hWnd);
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

			winGui_RemoveWindow(hWnd);

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
	_In_ WNDCLASSEX* lpWndClass, bool MDI_CHILD)
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
	pWinGui_Class->MDI_CHILD = MDI_CHILD;
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

HWND WINAPI WinGui_CreateWindowEx(
	_In_ DWORD dwExStyle,
	_In_opt_ const WCHAR* lpClassName,
	_In_opt_ const WCHAR* lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int iMonitor,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_ bool bClientSize,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LOGFONT* lpFont,
	_In_opt_ CLIENTCREATESTRUCT* lpCCS)
{
	HWND hWnd;
	WINGUI_WINDOW_INFO* virWin;
	WINGUI_WINDOW_INFO* virParentWin;
	int x, y, width, height;
	int monitorDpi;
	void* lpParam;

	if (lpClassName == 0)
		return 0;

	virWin = (WINGUI_WINDOW_INFO*)malloc(sizeof(WINGUI_WINDOW_INFO));
	if (virWin == 0) return 0;

	memset(virWin, 0, sizeof(WINGUI_WINDOW_INFO));

	virWin->x = X;
	virWin->y = Y;
	virWin->width = nWidth;
	virWin->height = nHeight;
	virWin->pClass = winGui_FindClass(lpClassName);
	virWin->iMonitor = iMonitor;
	virWin->bClientSize = bClientSize;

	virWin->lpFont = (LOGFONT*)malloc(sizeof(LOGFONT));
	if (virWin->lpFont)
	{
		if (lpFont)
			memcpy(virWin->lpFont, lpFont, sizeof(LOGFONT));
		else
		{
			memset(virWin->lpFont, 0, sizeof(LOGFONT));
			winGui_GetdefaultFont(virWin->lpFont, WINGUI_APPLICATION_DPI);
		}
	}

	lpParam = (lpCCS == 0) ? (void*)virWin : lpCCS;

	hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
		X, Y, nWidth, nHeight,
		hWndParent, hMenu, hInstance, virWin);

	if (hWnd == 0)
	{
		free(virWin);
		return 0;
	}

	virWin->handleResize = true;

	if (dwStyle & WS_CHILD)
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

		virWin->hWnd = hWnd;
		winGui_AddWindow(virWin);

		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)winGui_Control_WndProc);

		virWin->anchors = WINGUI_ANCHOR_TOP | WINGUI_ANCHOR_LEFT;

		virWin->monitorDpi = monitorDpi = winGui_GetDpiForWindow(hWnd);
		dpi_ResizeFont(hWnd, monitorDpi, virWin);

		if (hWndParent)
		{
			virParentWin = winGui_FindWindow(hWndParent);

			x = (int)dpi_MulDiv(X, monitorDpi, WINGUI_APPLICATION_DPI);
			y = (int)dpi_MulDiv(Y, monitorDpi, WINGUI_APPLICATION_DPI);
			width = (int)dpi_MulDiv(nWidth, monitorDpi, WINGUI_APPLICATION_DPI);
			height = (int)dpi_MulDiv(nHeight, monitorDpi, WINGUI_APPLICATION_DPI);

			virWin->x = x;
			virWin->y = y;
			virWin->width = width;
			virWin->height = height;

			//
			// Calculate offsets.
			//   We use the current client area, so WS_MAXIMIZE cannot be used.
			//   ShowWindow can be used to maximize & show the window, so this is not a problem.
			//

			RECT parentClientArea;

			GetClientRect(hWndParent, &parentClientArea);

			virWin->xOffset = (parentClientArea.right - parentClientArea.left) - x;
			virWin->yOffset = (parentClientArea.bottom - parentClientArea.top) - y;
			virWin->wOffset = (parentClientArea.right - parentClientArea.left) - (x + width);
			virWin->hOffset = (parentClientArea.bottom - parentClientArea.top) - (y + height);

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

bool WinGui_CenterWindow(HWND hWnd, ULONG centerFlags)
{
	HMONITOR hMonitor;
	MONITORINFO mInfo;
	WINGUI_WINDOW_INFO* virWin;
	RECT WinArea, ClientArea, parentRt, monitorRt;
	DWORD dwExStyle, style;
	LONG monitorWidth, monitorHeight;
	int xLoc, yLoc, thisWidth, thisHeight, parentWidth, parentHeight;
	int xExtra, yExtra, borderWidth, sysMenuBarHeight;
	HWND hWndParent;
	POINT xy;

	virWin = winGui_FindWindow(hWnd);
	if (virWin)
	{
		dwExStyle = (DWORD)GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		style = (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE);

		if ((centerFlags & WINGUI_CENTER_WINDOW) &&
			(!(centerFlags & WINGUI_CENTER_WITH_OWNER)) &&
			(!(style & WS_CHILD)) &&
			(!(dwExStyle & WS_EX_MDICHILD)))
		{
			hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			if (hMonitor == 0)
				return false;

			memset(&mInfo, 0, sizeof(MONITORINFO));
			mInfo.cbSize = sizeof(MONITORINFO);

			if (!GetMonitorInfo(hMonitor, &mInfo))
				return false;

			if (centerFlags & WINGUI_CENTER_OVER_WORKAREA)
				monitorRt = mInfo.rcWork;
			else
				monitorRt = mInfo.rcMonitor;

			monitorWidth = monitorRt.right - monitorRt.left;
			monitorHeight = monitorRt.bottom - monitorRt.top;

			if ((centerFlags & WINGUI_CENTER_CLIENT) == WINGUI_CENTER_CLIENT)
			{
				GetWindowRect(hWnd, &WinArea);
				GetClientRect(hWnd, &ClientArea);

				xExtra = WinArea.right - WinArea.left - ClientArea.right;
				yExtra = WinArea.bottom - WinArea.top - ClientArea.bottom;
				borderWidth = xExtra / 2;
				sysMenuBarHeight = yExtra - borderWidth;

				xLoc = monitorRt.left + ((monitorWidth - (ClientArea.right - ClientArea.left)) / 2) - borderWidth;
				yLoc = monitorRt.top + ((monitorHeight - (ClientArea.bottom - ClientArea.top)) / 2) - sysMenuBarHeight;
			}
			else
			{
				GetWindowRect(hWnd, &WinArea);

				xLoc = monitorRt.left + ((monitorWidth - (WinArea.right - WinArea.left)) / 2);
				yLoc = monitorRt.top + ((monitorHeight - (WinArea.bottom - WinArea.top)) / 2);
			}

			if (SetWindowPos(hWnd, NULL, xLoc, yLoc, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE))
				return true;
		}
		else if (centerFlags & WINGUI_CENTER_WINDOW) // Center using the parent/owner
		{
			hWndParent = GetParent(hWnd);
			if (hWndParent)
			{
				GetWindowRect(hWndParent, &parentRt);

				if (style & WS_CHILD)
				{
					xy.x = parentRt.left;
					xy.y = parentRt.top;
					ScreenToClient(hWndParent, &xy);
					parentRt.left = xy.x;
					parentRt.top = xy.y;

					xy.x = parentRt.right;
					xy.y = parentRt.bottom;
					ScreenToClient(hWndParent, &xy);
					parentRt.right = xy.x;
					parentRt.bottom = xy.y;
				}
				else
				{
					// If the owner is on a different monitor, move it first for DPI changes to take effect.

					HMONITOR src_hMonitor;
					MONITORINFO src_mInfo;

					hMonitor = MonitorFromWindow(hWndParent, MONITOR_DEFAULTTONEAREST);
					if (hMonitor == 0)
						return false;

					src_hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
					if (src_hMonitor == 0)
						return false;

					memset(&mInfo, 0, sizeof(MONITORINFO));
					mInfo.cbSize = sizeof(MONITORINFO);

					if (!GetMonitorInfo(hMonitor, &mInfo))
						return false;

					memset(&src_mInfo, 0, sizeof(MONITORINFO));
					src_mInfo.cbSize = sizeof(MONITORINFO);

					if (!GetMonitorInfo(src_hMonitor, &src_mInfo))
						return false;

					if ((mInfo.rcMonitor.left != src_mInfo.rcMonitor.left) ||
						(mInfo.rcMonitor.right != src_mInfo.rcMonitor.right) ||
						(mInfo.rcMonitor.top != src_mInfo.rcMonitor.top) ||
						(mInfo.rcMonitor.bottom != src_mInfo.rcMonitor.bottom))
					{
						if (!SetWindowPos(hWnd, NULL, mInfo.rcMonitor.left, mInfo.rcMonitor.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE))
							return false;
					}
				}

				if ((centerFlags & WINGUI_CENTER_CLIENT) == WINGUI_CENTER_CLIENT)
				{
					GetWindowRect(hWnd, &WinArea);
					GetClientRect(hWnd, &ClientArea);

					xExtra = WinArea.right - WinArea.left - ClientArea.right;
					yExtra = WinArea.bottom - WinArea.top - ClientArea.bottom;
					borderWidth = xExtra / 2;
					sysMenuBarHeight = yExtra - borderWidth;
				}
				else
				{
					GetWindowRect(hWnd, &WinArea);

					borderWidth = 0;
					sysMenuBarHeight = 0;
				}

				thisWidth = WinArea.right - WinArea.left;
				thisHeight = WinArea.bottom - WinArea.top;

				parentWidth = parentRt.right - parentRt.left;
				parentHeight = parentRt.bottom - parentRt.top;

				xLoc = parentRt.left + ((parentWidth - thisWidth) / 2) - borderWidth;
				yLoc = parentRt.top + ((parentHeight - thisHeight) / 2) - sysMenuBarHeight;

				if (SetWindowPos(hWnd, NULL, xLoc, yLoc, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE))
					return true;
			}
		}
	}

	return false;
}
