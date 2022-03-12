
#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

// Version # 1.0

#define WINGUI_VERSION    10

// Flag Options for WinGui_Init

#define WINGUI_FLAGS_NONE                 0
#define WINGUI_FLAGS_REPORT_ERRORS        1

// DPI for the entire application.

#define WINGUI_APPLICATION_DPI            96

// define WM_DPICHANGED when it doesn't exist.

#ifndef WM_DPICHANGED
#define WM_DPICHANGED  0x02E0
#endif

// define GUI_PROCESS_DPI_AWARENESS for when PROCESS_DPI_AWARENESS doesn't exist.

typedef enum GUI_PROCESS_DPI_AWARENESS {
	GUI_PROCESS_DPI_UNAWARE = 0,
	GUI_PROCESS_SYSTEM_DPI_AWARE = 1,
	GUI_PROCESS_PER_MONITOR_DPI_AWARE = 2
} GUI_PROCESS_DPI_AWARENESS;

// define GUI_DPI_AWARENESS_CONTEXT for when DPI_AWARENESS_CONTEXT doesn't exist.

DECLARE_HANDLE(GUI_DPI_AWARENESS_CONTEXT);

#define GUI_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((GUI_DPI_AWARENESS_CONTEXT)-2)
#define GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((GUI_DPI_AWARENESS_CONTEXT)-3)
#define GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((GUI_DPI_AWARENESS_CONTEXT)-4)

// DPI function pointer types.

typedef UINT(WINAPI* GetDpiForSystem_proc)();
typedef UINT(WINAPI* GetDpiForWindow_proc)(HWND hWnd);
typedef BOOL(WINAPI* SetProcessDPIAware_proc)();
typedef HRESULT(WINAPI* SetProcessDpiAwareness_proc)(GUI_PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI* SetProcessDpiAwarenessContext_proc)(GUI_DPI_AWARENESS_CONTEXT value);
typedef BOOL(WINAPI* EnableNonClientDpiScaling_proc)(HWND hWnd);
typedef BOOL(WINAPI* AdjustWindowRectExForDpi_proc)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

// Anchor flag options.

#define ANCHOR_TOP       1
#define ANCHOR_BOTTOM    2
#define ANCHOR_LEFT      4
#define ANCHOR_RIGHT     8

// Class info for windows.

typedef struct _WINGUI_CLASS_INFO WINGUI_CLASS_INFO;
struct _WINGUI_CLASS_INFO
{
	WCHAR* className;
	WNDPROC WndProc;
	WINGUI_CLASS_INFO* next;
};

// Window virtual info

typedef struct _WIN_VIR_INFO WIN_VIR_INFO;
struct _WIN_VIR_INFO
{
	int x;
	int y;
	int width;
	int height;
	long monitorDpi;
	WINGUI_CLASS_INFO* pClass;
	LOGFONT* lpFont;
	unsigned long anchors;
	long xOffset;
	long yOffset;
	long wOffset;
	long hOffset;
	void* userData;
	bool handleResize;
};

// WinGui Functions.

//
// Initialize WinGui
//
// flags parameter options:
// WINGUI_FLAGS_NONE
// WINGUI_FLAGS_REPORT_ERRORS
//
// Returns true for acceptable success, false for a fatal error
//
bool WinGui_Init(HINSTANCE hInstance, unsigned long flags);

//
// Set Control Anchors
//
// anchors parameter options:
// ANCHOR_TOP
// ANCHOR_BOTTOM
// ANCHOR_LEFT
// ANCHOR_RIGHT
//
// Returns true for success, false otherwise
//
bool WINAPI WinGui_SetAnchors(HWND hWnd, unsigned long anchors);

//
// Register Class
// This is an overlay for windows RegisterClassEx function.
//
ATOM
WINAPI
WinGui_RegisterClassEx(
	_In_ WNDCLASSEX* lpWndClass);

//
// Unregister Class
// This is an overlay for windows UnregisterClass function.
//
BOOL WinGui_UnregisterClass(
	_In_ const WCHAR* lpClassName,
	_In_opt_ HINSTANCE hInstance);

//
// Create Window
// This is an overlay for windows CreateWindowEx function.
//
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
	_In_opt_ LOGFONT* lpFont);

//
// Set User Data
// This function must be used in place of SetWindowLongPtr(hWnd, GWLP_USERDATA)
//
// Returns true for success, false otherwise
//
bool WinGui_SetWindowUserData(HWND hWnd, void* UserData);

//
// Get User Data
// This function must be used in place of GetWindowLongPtr(hWnd, GWLP_USERDATA)
//
// Returns the user data, 0 otherwise
//
void* WinGui_GetWindowUserData(HWND hWnd);
