
#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

// Version # 1.0

#define WINGUI_VERSION                    10

// Flag Options for WinGui_Init

#define WINGUI_FLAGS_NONE                 0
#define WINGUI_FLAGS_REPORT_ERRORS        1

// DPI for the entire application.

#define WINGUI_APPLICATION_DPI            96

// Monitor options.

#define WINGUI_MONITOR_NORMAL             0x00000000
#define WINGUI_MONITOR_DEFAULT            0x10000000
#define WINGUI_MONITOR_FROM_MOUSE         0x20000000

// Center flag options.

#define WINGUI_CENTER_WINDOW              0x10000
#define WINGUI_CENTER_CLIENT              0x30000
#define WINGUI_CENTER_OVER_WORKAREA       0x40000
#define WINGUI_CENTER_WITH_OWNER          0x80000

// Anchor flag options.

#define WINGUI_ANCHOR_TOP                 1
#define WINGUI_ANCHOR_BOTTOM              2
#define WINGUI_ANCHOR_LEFT                4
#define WINGUI_ANCHOR_RIGHT               8

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

// define GUI_MONITOR_DPI_TYPE for when MONITOR_DPI_TYPE doesn't exist.

typedef enum GUI_MONITOR_DPI_TYPE {
	GUI_MDT_EFFECTIVE_DPI = 0,
	GUI_MDT_ANGULAR_DPI = 1,
	GUI_MDT_RAW_DPI = 2,
	GUI_MDT_DEFAULT
} GUI_MONITOR_DPI_TYPE;

// define GUI_DPI_AWARENESS_CONTEXT for when DPI_AWARENESS_CONTEXT doesn't exist.

DECLARE_HANDLE(GUI_DPI_AWARENESS_CONTEXT);

#define GUI_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((GUI_DPI_AWARENESS_CONTEXT)-2)
#define GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((GUI_DPI_AWARENESS_CONTEXT)-3)
#define GUI_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((GUI_DPI_AWARENESS_CONTEXT)-4)

// DPI function pointer types.

typedef HRESULT(WINAPI* GetDpiForMonitor_proc)(HMONITOR hmonitor, GUI_MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
typedef UINT(WINAPI* GetDpiForSystem_proc)();
typedef UINT(WINAPI* GetDpiForWindow_proc)(HWND hWnd);
typedef BOOL(WINAPI* SetProcessDPIAware_proc)();
typedef HRESULT(WINAPI* SetProcessDpiAwareness_proc)(GUI_PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI* SetProcessDpiAwarenessContext_proc)(GUI_DPI_AWARENESS_CONTEXT value);
typedef BOOL(WINAPI* EnableNonClientDpiScaling_proc)(HWND hWnd);
typedef BOOL(WINAPI* AdjustWindowRectExForDpi_proc)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

// WinGui class info.

typedef struct _WINGUI_CLASS_INFO WINGUI_CLASS_INFO;
struct _WINGUI_CLASS_INFO
{
	WCHAR* className;
	bool MDI_CHILD;
	WNDPROC WndProc;
	WINGUI_CLASS_INFO* next;
};

// WinGui window info.

typedef struct _WINGUI_WINDOW_INFO WINGUI_WINDOW_INFO;
struct _WINGUI_WINDOW_INFO
{
	HWND hWnd;
	int x;
	int y;
	int width;
	int height;
	long monitorDpi;
	WINGUI_CLASS_INFO* pClass;
	bool bClientSize;
	int iMonitor;
	LOGFONT* lpFont;
	unsigned long anchors;
	long xOffset;
	long yOffset;
	long wOffset;
	long hOffset;
	void* userData;
	bool handleResize;
	WINGUI_WINDOW_INFO* next;
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
// Register Class
// This is an overlay for windows RegisterClassEx function.
//
ATOM
WINAPI
WinGui_RegisterClassEx(
	_In_ WNDCLASSEX* lpWndClass, bool MDI_CHILD);

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
	_In_opt_ CLIENTCREATESTRUCT* lpCCS);

//
// Set Control Anchors
//
// anchors flags:
// WINGUI_ANCHOR_TOP
// WINGUI_ANCHOR_BOTTOM
// WINGUI_ANCHOR_LEFT
// WINGUI_ANCHOR_RIGHT
//
// Returns true for success, false otherwise
//
bool WINAPI WinGui_SetAnchors(HWND hWnd, ULONG anchors);

//
// Center a window
//
// centerFlags flags:
// WINGUI_CENTER_WINDOW
// WINGUI_CENTER_CLIENT
// WINGUI_CENTER_OVER_WORKAREA
// WINGUI_CENTER_WITH_OWNER
//
// Returns true for success, false otherwise
//
bool WinGui_CenterWindow(HWND hWnd, ULONG centerFlags);
