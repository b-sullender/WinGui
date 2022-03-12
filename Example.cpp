
#include "WinGui.h"
#include "resource.h"

#define WinClassName L"WinGui Example App"
#define WinTitle L"WinGui Example App"

static HWND mainWin_hWnd;
static HWND button1_hWnd;
static HWND listview1_hWnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC WinDC;
	PAINTSTRUCT ps;
	int wmId;
	int wmEvent;

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PRINTCLIENT:
	{
		WinDC = (HDC)wParam;

		// Code goes here ...

		return 0;
	}
	case WM_PAINT:
	{
		WinDC = BeginPaint(hWnd, &ps);

		// Code goes here ...

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmEvent)
		{
		case BN_CLICKED:
		{
			// Code goes here ...

			break;
		}
		default:
			break;
		}

		break;
	}
	case WM_NOTIFY:
	{
		// Code goes here ...

		break;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ WCHAR* pCmdLine, _In_ int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;

	if (!WinGui_Init(hInstance, WINGUI_FLAGS_REPORT_ERRORS))
		return 0;

	// Register the main window.

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WinClassName;
	wcex.hIconSm = NULL;

	if (!WinGui_RegisterClassEx(&wcex))
	{
		MessageBox(0, L"Failed to register the main window class.", L"Error!", MB_OK);
		return 0;
	}

	// Create the main window.

	mainWin_hWnd = WinGui_CreateWindow(WS_EX_OVERLAPPEDWINDOW, WinClassName, WinTitle,
		WS_VISIBLE | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
		0, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 450, true, NULL, NULL, hInstance, NULL, NULL);

	if (!mainWin_hWnd)
	{
		MessageBox(0, L"Failed to create the main window.", L"Error!", MB_OK);
		return 0;
	}

	// Create some buttons.

	button1_hWnd = WinGui_CreateWindow(NULL, L"BUTTON", L"Button1", WS_VISIBLE | WS_CHILD,
		0, 12, 12, 68, 24, false, mainWin_hWnd, NULL, hInstance, NULL, NULL);
	if (!button1_hWnd) return 0;

	// Create a listview.

	listview1_hWnd = WinGui_CreateWindow(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_VISIBLE | WS_VSCROLL | WS_CHILD | LVS_REPORT,
		0, 12, 44, 976, 200, true, mainWin_hWnd, NULL, hInstance, NULL, NULL);
	if (!listview1_hWnd) return 0;

	// Set extended listview styles.

	ListView_SetExtendedListViewStyle(listview1_hWnd, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

	// Add some columns to our listview.

	WCHAR ColumnHeadText1[] = L"Column1";
	WCHAR ColumnHeadText2[] = L"Column2";
	WCHAR ColumnHeadText3[] = L"Column3";

	LVCOLUMN Lvc;

	Lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	Lvc.fmt = LVCFMT_LEFT;
	Lvc.cx = 100;
	Lvc.pszText = ColumnHeadText1;
	Lvc.iSubItem = 0;
	if (ListView_InsertColumn(listview1_hWnd, 0, &Lvc) == -1) return FALSE;

	Lvc.cx = 100;
	Lvc.pszText = ColumnHeadText2;
	Lvc.iSubItem = 1;
	if (ListView_InsertColumn(listview1_hWnd, 1, &Lvc) == -1) return FALSE;

	Lvc.cx = 115;
	Lvc.pszText = ColumnHeadText3;
	Lvc.iSubItem = 2;
	if (ListView_InsertColumn(listview1_hWnd, 2, &Lvc) == -1) return FALSE;

	// Add some items to our listview.

	LVITEM lvi;

	for (ULONG i = 0; i < 99; i++)
	{
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = L"This is";
		ListView_InsertItem(listview1_hWnd, &lvi);
		ListView_SetItemText(listview1_hWnd, i, 1, L"some items text");
		ListView_SetItemText(listview1_hWnd, i, 2, L"in a listview");
	}

	// Set text color for the listview.

	ListView_SetTextColor(listview1_hWnd, RGB(255, 0, 0));

	// Set anchors for some controls.

	WinGui_SetAnchors(listview1_hWnd, ANCHOR_TOP | ANCHOR_BOTTOM | ANCHOR_LEFT | ANCHOR_RIGHT);

	// Main message loop.

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
