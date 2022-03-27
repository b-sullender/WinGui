# WinGui_SetAnchors

Sets anchors for a window.

## Syntax

```
bool WINAPI WinGui_SetAnchors(HWND hWnd, ULONG anchors);
```

## Parameters

```
[in] hWnd
```
Type: **HWND**

A handle to the window to set the anchors.

```
[in] anchors
```
Type: **ULONG**

The anchors to set. This can be any combination of the following flags.

| Flag | Description |
| --- | --- |
| WINGUI_ANCHOR_TOP | Top side of the window is anchored |
| WINGUI_ANCHOR_BOTTOM | Bottom side of the window is anchored |
| WINGUI_ANCHOR_LEFT | Left side of the window is anchored |
| WINGUI_ANCHOR_RIGHT | Right side of the window is anchored |

## Return value

Returns true on success, false otherwise.

## Requirements

- WinGui Version 1.0
