# WinGui_Init

Initializes WinGui

## Syntax

```
bool WinGui_Init(HINSTANCE hInstance, unsigned long flags);
```

## Parameters

```
[in] hInstance
```
Type: **HINSTANCE**

A handle to the executable that is using WinGui. This should be the same hInstance that is passed to the wWinMain entry point.

```
[in] flags
```
Type: **unsigned long**

This parameter can be one of the following flags.

| Flag | Description |
| --- | --- |
| WINGUI_FLAGS_NONE | Nothing is done when WinGui succeeds or fails to initialize |
| WINGUI_FLAGS_REPORT_ERRORS | Fatal & Non-Fatal errors are reported to the user via MessageBox |

## Return value

Returns true on success, false otherwise.

## Remarks

This should be the first function an application calls in its wWinMain entry point, this allows subsequent calls to the MessageBox function to have proper DPI scaling.

Only returns false for fatal errors.

## Requirements

- WinGui Version 1.0
