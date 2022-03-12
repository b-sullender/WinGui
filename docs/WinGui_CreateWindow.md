# WinGui_CreateWindow

Creates an overlapped, pop-up, or child window with an extended window style.

## Syntax

```
HWND WINAPI WinGui_CreateWindow(
	_In_ DWORD dwExStyle,
	_In_opt_ const WCHAR* lpClassName,
	_In_opt_ const WCHAR* lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int StartPosition,
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
```

## Parameters

```
[in] dwExStyle
```
Type: **DWORD**

```
[in] lpClassName
```
Type: **const WCHAR\***

```
[in] lpWindowName
```
Type: **const WCHAR\***

```
[in] dwStyle
```
Type: **DWORD**

```
[in] StartPosition
```
Type: **int**

```
[in] X
```
Type: **int**

```
[in] Y
```
Type: **int**

```
[in] nWidth
```
Type: **int**

```
[in] nHeight
```
Type: **int**

```
[in] bClientSize
```
Type: **bool**

```
[in] hWndParent
```
Type: **HWND*

```
[in] hMenu
```
Type: **HMENU**

```
[in] hInstance
```
Type: **HINSTANCE**

```
[in] lpParam
```
Type: **LPVOID**

```
[in] lpFont
```
Type: **LOGFONT\***

## Return value

If the function succeeds, the return value is a handle to the new window.

If the function fails, the return value is NULL. To get extended error information, call [GetLastError](https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror).

## Remarks

This function is a layer for [CreateWindowEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw).
This layer adds 3 new parameters **StartPosition**, **bClientSize** and **lpFont**

## Requirements

- WinGui Version 1.0
