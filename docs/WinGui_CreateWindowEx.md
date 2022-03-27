# WinGui_CreateWindowEx

Creates an overlapped, pop-up, or child window with an extended window style.

## Syntax

```
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
```

## Parameters

```
[in] dwExStyle
```
Type: **DWORD**

The extended window style of the window being created. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] lpClassName
```
Type: **const WCHAR\***

A null-terminated string or a class atom created by a previous call to the [WinGui_RegisterClassEx](docs/WinGui_RegisterClassEx.md) function. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] lpWindowName
```
Type: **const WCHAR\***

The window name. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] dwStyle
```
Type: **DWORD**

The style of the window being created. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] iMonitor
```
Type: **int**

Defines the start monitor.

One of the following flags can be used.

| Flag | Description |
| --- | --- |
| WINGUI_MONITOR_NORMAL | The start monitor is defined by the default behavior |
| WINGUI_MONITOR_DEFAULT | The start monitor is the primary display monitor. |
| WINGUI_MONITOR_FROM_MOUSE | The start monitor is retrieved from the mouse location |

```
[in] X
```
Type: **int**

The initial virtual horizontal position of the window. For an overlapped or pop-up window, the x parameter is the initial virtual x-coordinate of the window's upper-left corner, in screen coordinates. For a child window, x is the virtual x-coordinate of the upper-left corner of the child window relative to the upper-left corner of the parent window's client area.


For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] Y
```
Type: **int**

The initial virtual vertical position of the window. For an overlapped or pop-up window, the y parameter is the initial virtual y-coordinate of the window's upper-left corner, in screen coordinates. For a child window, y is the virtual y-coordinate of the upper-left corner of the child window relative to the upper-left corner of the parent window's client area.


For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] nWidth
```
Type: **int**

The virtual width of the window. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] nHeight
```
Type: **int**

The virtual height of the window. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] bClientSize
```
Type: **bool**

When this parameter is set to true, paramters nWidth and nHeight is the virtual size of the client area of the window. Otherwise its the virtual size of the entire window.

```
[in] hWndParent
```
Type: **HWND*

A handle to the parent or owner window of the window being created. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] hMenu
```
Type: **HMENU**

A handle to a menu, or specifies a child-window identifier, depending on the window style. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] hInstance
```
Type: **HINSTANCE**

A handle to the instance of the module to be associated with the window. For more info about this parameter see [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).

```
[in] lpFont
```
Type: **LOGFONT\***

A pointer to a [LOGFONT](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw) structure that defines the characteristics of the logical font for the window.

```
[in] lpCCS
```
Type: **CLIENTCREATESTRUCT\***

A pointer to a [CLIENTCREATESTRUCT](https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-clientcreatestruct) structure that contains information about the menu and first multiple-document interface (MDI) child window of an MDI client window.

## Return value

If the function succeeds, the return value is a handle to the new window.

If the function fails, the return value is NULL. To get extended error information, call [GetLastError](https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror).

## Remarks

This function is a layer for [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw). This layer adds 3 new parameters **iMonitor**, **bClientSize**, **lpFont**, and replaces lpParam with lpCCS.

An application should use WinGui_CreateWindowEx with the WS_EX_MDICHILD style to create a MDI child window. Using SendMessage(WM_MDICREATE) with a MDICREATESTRUCT struct will cause a crash because WinGui uses the lParam paramter.

## Requirements

- WinGui Version 1.0
