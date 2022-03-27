
#include "WinGui.h"
#include "resource.h"

#define WinClassName L"WinGui_Window"
#define WinTitle L"WinGui Example Window"

#define WinMDI_ClassName L"WinGui_MDI_Window"
#define WinMDI_Title L"WinGui MDI Example Window"

#define WinMDI_ChildClassName L"WinGui_MDI_Child_Window"
#define WinMDI_ChildTitle L"WinGui MDI Child Window"

#define WinDialog_ClassName L"Dialog_Window"
#define WinDialog_Title L"Dialog Window"

static HWND mainWin_hWnd;
static HWND MDICLIENT_hWnd;
static HWND DocWin1_hWnd;
static HWND DialogWin_hWnd;

static HWND button1_hWnd;
static HWND textbox1_hWnd;
static HWND listview1_hWnd;

LRESULT CALLBACK Dialog_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		DestroyWindow(hWnd);
		break;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

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

			if (wmId == IDI_BUTTON1)
			{
				MessageBox(NULL, L"Button1 Clicked!", L"OK!", MB_OK);
			}
			else if (wmId == ID_MENU_SAVE_FILE)
			{
				DialogWin_hWnd = WinGui_CreateWindowEx(0, WinDialog_ClassName, WinDialog_Title,
					WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN,
					0, 0, 0, 800, 200, true, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL, NULL);

				WinGui_CenterWindow(DialogWin_hWnd, WINGUI_CENTER_WINDOW | WINGUI_CENTER_WITH_OWNER);
				ShowWindow(DialogWin_hWnd, SW_SHOWNORMAL);
			}

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

LRESULT CALLBACK MDI_Frame_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

			if (wmId == ID_MENU_SAVE_FILE)
			{
				DialogWin_hWnd = WinGui_CreateWindowEx(0, WinDialog_ClassName, WinDialog_Title,
					WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN,
					0, 0, 0, 400, 200, true, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL, NULL);

				WinGui_CenterWindow(DialogWin_hWnd, WINGUI_CENTER_WINDOW | WINGUI_CENTER_WITH_OWNER);
				ShowWindow(DialogWin_hWnd, SW_SHOWNORMAL);
			}

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
	case WM_CREATE:
	{
		CLIENTCREATESTRUCT ccs;

		ccs.hWindowMenu = 0;
		ccs.idFirstChild = IDM_MDI_FIRST_CHILD;

		MDICLIENT_hWnd = WinGui_CreateWindowEx(0, L"MDICLIENT", NULL,
			WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
			0, 0, 0, 1000, 450, false, hWnd, NULL,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL, &ccs);

		WinGui_SetAnchors(MDICLIENT_hWnd,
			WINGUI_ANCHOR_TOP | WINGUI_ANCHOR_BOTTOM | WINGUI_ANCHOR_LEFT | WINGUI_ANCHOR_RIGHT);

		ShowWindow(MDICLIENT_hWnd, SW_SHOW);

		break;
	}
	default:
		break;
	}

	// Note: A MDI Frame window uses DefFrameProc instead of the normal DefWindowProc function.

	return DefFrameProc(hWnd, MDICLIENT_hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MDI_Child_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC WinDC;
	PAINTSTRUCT ps;
	int wmId;
	int wmEvent;

	switch (uMsg)
	{
	case WM_DESTROY:
		DestroyWindow(hWnd);
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

			if (wmId == IDI_BUTTON1)
			{
				MessageBox(NULL, L"Button1 Clicked!", L"OK!", MB_OK);
			}

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

	// Note: A MDI Child window uses DefMDIChildProc instead of the normal DefWindowProc function.

	return DefMDIChildProc(hWnd, uMsg, wParam, lParam);
}

bool addWindowControls(HWND hWndParent, HINSTANCE hInstance)
{
	// Create some buttons.

	button1_hWnd = WinGui_CreateWindowEx(NULL, L"BUTTON", L"Button1", WS_VISIBLE | WS_CHILD,
		0, 12, 12, 68, 24, false, hWndParent, (HMENU)IDI_BUTTON1, hInstance, NULL, NULL);
	if (!button1_hWnd) return false;

	// Create a textbox.

	textbox1_hWnd = WinGui_CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0.0", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
		0, 88, 14, 120, 20, false, hWndParent, (HMENU)IDI_TEXTBOX1, hInstance, NULL, NULL);
	if (!textbox1_hWnd) return false;

	// Create a listview.

	listview1_hWnd = WinGui_CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_VISIBLE | WS_VSCROLL | WS_CHILD | LVS_REPORT,
		0, 12, 44, 576, 200, true, hWndParent, (HMENU)IDI_LISTVIEW1, hInstance, NULL, NULL);
	if (!listview1_hWnd) return false;

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
	if (ListView_InsertColumn(listview1_hWnd, 0, &Lvc) == -1) return false;

	Lvc.cx = 100;
	Lvc.pszText = ColumnHeadText2;
	Lvc.iSubItem = 1;
	if (ListView_InsertColumn(listview1_hWnd, 1, &Lvc) == -1) return false;

	Lvc.cx = 115;
	Lvc.pszText = ColumnHeadText3;
	Lvc.iSubItem = 2;
	if (ListView_InsertColumn(listview1_hWnd, 2, &Lvc) == -1) return false;

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

	//
	// Set anchors for controls.
	//   Note, WinGui_SetAnchors needs to be called before calling ShowWindow.
	//

	WinGui_SetAnchors(listview1_hWnd,
		WINGUI_ANCHOR_TOP | WINGUI_ANCHOR_BOTTOM | WINGUI_ANCHOR_LEFT | WINGUI_ANCHOR_RIGHT);

	// return success

	return true;
}

bool WINAPI RunMDIexample(HINSTANCE hInstance, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;

	// Register the Main Frame window class.

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MDI_Frame_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WinMDI_ClassName;
	wcex.hIconSm = NULL;

	if (!WinGui_RegisterClassEx(&wcex, false))
	{
		MessageBox(0, L"Failed to register the MDI Frame window class.", L"Error!", MB_OK);
		return false;
	}

	// Register the MDI child window class.

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MDI_Child_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WinMDI_ChildClassName;
	wcex.hIconSm = NULL;

	if (!WinGui_RegisterClassEx(&wcex, true))
	{
		MessageBox(0, L"Failed to register the MDI Child window class.", L"Error!", MB_OK);
		return false;
	}

	// Load the main window menu resource.

	HMENU hMainWindowMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));

	// Create the Frame window.

	mainWin_hWnd = WinGui_CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, WinMDI_ClassName, WinMDI_Title,
		WS_BORDER | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		WINGUI_MONITOR_FROM_MOUSE, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 450, true, NULL, hMainWindowMenu, hInstance, NULL, NULL);

	if (!mainWin_hWnd)
	{
		MessageBox(0, L"Failed to create the MDI Frame window.", L"Error!", MB_OK);
		return false;
	}

	//
	// Create the child (document) window.
	//   Note, An application should use WinGui_CreateWindowEx with the WS_EX_MDICHILD style to create a MDI child window.
	//   Using SendMessage(WM_MDICREATE) with a MDICREATESTRUCT struct will cause a crash because WinGui uses the lParam paramter.
	//

	DocWin1_hWnd = WinGui_CreateWindowEx(WS_EX_MDICHILD, WinMDI_ChildClassName, L"Document Window",
		WS_BORDER | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		0, 0, 0, 600, 300, true, MDICLIENT_hWnd, NULL, hInstance, NULL, NULL);

	if (!DocWin1_hWnd)
	{
		MessageBox(0, L"Failed to create the MDI Child (Document) window.", L"Error!", MB_OK);
		return false;
	}

	// Add all the controls to the MDI Child window.

	addWindowControls(DocWin1_hWnd, hInstance);

	//
	// Show the MDI Frame window.
	//   Note, that CreateWindowEx does not handle some window styles appropriately.
	//   It expects ShowWindow to be called with the nCmdShow parameter thats passed in wWinMain.
	//   For this reason we don't use WS_MINIMIZE or WS_MAXIMIZE window styles.
	//

	ShowWindow(mainWin_hWnd, SW_SHOWMAXIMIZED);

	// We maximized the MDI Frame window when we called ShowWindow, so lets re-center the child window.

	WinGui_CenterWindow(DocWin1_hWnd, WINGUI_CENTER_CLIENT);

	// Show the MDI Child window.

	ShowWindow(DocWin1_hWnd, SW_SHOW);

	// Main message loop.

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

bool RunExample(HINSTANCE hInstance, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;

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

	if (!WinGui_RegisterClassEx(&wcex, false))
	{
		MessageBox(0, L"Failed to register the main window class.", L"Error!", MB_OK);
		return false;
	}

	// Load the main window menu resource.

	HMENU hMainWindowMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));

	// Create the main window.

	mainWin_hWnd = WinGui_CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, WinClassName, WinTitle,
		WS_BORDER | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		WINGUI_MONITOR_FROM_MOUSE, 0, 0, 600, 300, true, NULL, hMainWindowMenu, hInstance, NULL, NULL);

	if (!mainWin_hWnd)
	{
		MessageBox(0, L"Failed to create the main window.", L"Error!", MB_OK);
		return false;
	}

	// Center the main window.

	WinGui_CenterWindow(mainWin_hWnd, WINGUI_CENTER_CLIENT);

	// Add all the controls to the MDI Child window.

	addWindowControls(mainWin_hWnd, hInstance);

	//
	// Show the window.
	//   Note, that CreateWindowEx does not handle some window styles appropriately.
	//   It expects ShowWindow to be called with the nCmdShow parameter thats passed in wWinMain.
	//   For this reason we don't use WS_MINIMIZE or WS_MAXIMIZE window styles.
	//

	ShowWindow(mainWin_hWnd, nCmdShow);

	// Main message loop.

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ WCHAR* pCmdLine, _In_ int nCmdShow)
{
	if (!WinGui_Init(hInstance, WINGUI_FLAGS_REPORT_ERRORS))
	{
		MessageBox(0, L"WinGui failed to initialize, exiting the app!", L"Error!", MB_OK);
		return 0;
	}

	WNDCLASSEX wcex;

	// Register the dialog window.

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Dialog_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WinDialog_ClassName;
	wcex.hIconSm = NULL;

	if (!WinGui_RegisterClassEx(&wcex, false))
	{
		MessageBox(0, L"Failed to register the dialog window class.", L"Error!", MB_OK);
		return 0;
	}

	RunMDIexample(hInstance, nCmdShow);
	//RunExample(hInstance, nCmdShow);

	return 0;
}
