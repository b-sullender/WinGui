# WinGui_RegisterClassEx

Registers a window class for subsequent use in calls to the WinGui_CreateWindow function.

## Syntax

```
ATOM
WINAPI
WinGui_RegisterClassEx(
	_In_ WNDCLASSEX* lpWndClass);
```

## Parameters

```
[in] lpWndClass
```
Type: **WNDCLASSEX\***

A pointer to a [WNDCLASSEX](https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw) structure. You must fill the structure with the appropriate class attributes before passing it to the function.

## Return value

If the function succeeds, the return value is a class atom that uniquely identifies the class being registered. If the function fails, the return value is zero. To get extended error information, call [GetLastError](https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror).

## Remarks

This function is a layer for [RegisterClassEx](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw)

## Requirements

|                      |
| --- | --- |
| WinGui Version | 1.0 |
