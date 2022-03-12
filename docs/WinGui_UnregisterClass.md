# WinGui_UnregisterClass

Unregisters a window class, freeing the memory required for the class.

## Syntax

```
BOOL WinGui_UnregisterClass(
	_In_ const WCHAR* lpClassName,
	_In_opt_ HINSTANCE hInstance);
```

## Parameters

```
[in] lpClassName
```
Type: **const WCHAR\***

A null-terminated string or a class atom. If lpClassName is a string, it specifies the window class name. This class name must have been registered by a previous call to the [WinGui_RegisterClassEx](docs/WinGui_RegisterClassEx.md) function.

```
[in] hInstance
```
Type: **HINSTANCE**

A handle to the instance of the module that created the class.

## Return value

If the function succeeds, the return value is nonzero.

If the class could not be found or if a window still exists that was created with the class, the return value is zero. To get extended error information, call [GetLastError](https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror).

## Remarks

This function is a layer for [UnregisterClassW](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-unregisterclassw).

## Requirements

|                      |
| --- | --- |
| WinGui Version | 1.0 |
