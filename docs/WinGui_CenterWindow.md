# WinGui_CenterWindow

Centers a window relative to a monitor, parent window, or a owner window.

## Syntax

```
bool WinGui_CenterWindow(HWND hWnd, ULONG centerFlags);
```

## Parameters

```
[in] hWnd
```
Type: **HWND**

A handle to the window to center.

```
[in] centerFlags
```
Type: **ULONG**

Center flags.

| Flag | Description |
| --- | --- |
| WINGUI_CENTER_WINDOW | The entire window is centered |
| WINGUI_CENTER_CLIENT | The client area of the window is centered |
| WINGUI_CENTER_OVER_WORKAREA | The top-level window will be centered in the monitor work area |
| WINGUI_CENTER_WITH_OWNER | The window is centered with its owner |

When the window being centered has the flag WS_CHILD or WS_EX_MDICHILD the window is centered relative to the parent window, not the monitor. For non-child windows WINGUI_CENTER_WINDOW or WINGUI_CENTER_CLIENT can be combined with WINGUI_CENTER_WITH_OWNER to center a window relative to the owner window, not the monitor.

## Return value

Returns true on success, false otherwise.

## Requirements

- WinGui Version 1.0
