// Server.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Server.h"


#include <winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

int nPort = 5555;

HWND hEditIn = NULL;
HWND hEditOut = NULL;
SOCKET Socket = NULL;
char szHistory[10000];
sockaddr sockAddrClient;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   0, 0, 530, 600, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;

	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		{
			ZeroMemory(szHistory, sizeof(szHistory));
			
			//create history label
			CreateWindowEx(WS_EX_CLIENTEDGE,
				"STATIC",
				"History, Context Receive From Client",
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				50,
				20,
				400,
				20,
				hWnd,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			// Create incoming message box
			hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE,
					"EDIT",
					"",
					WS_CHILD | WS_VISIBLE | ES_MULTILINE |
					ES_AUTOVSCROLL | ES_AUTOHSCROLL,
					50,
					50,
					400,
					200,
					hWnd,
					(HMENU)IDC_EDIT_IN,
					GetModuleHandle(NULL),
					NULL);
			if (!hEditIn)
			{
				MessageBox(hWnd,
						"Could not create incoming edit box.",
						"Error",
						MB_OK | MB_ICONERROR);
			}
			HGDIOBJ hfDefault = GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEditIn,
					  WM_SETFONT,
					  (WPARAM)hfDefault,
					  MAKELPARAM(FALSE, 0));
			SendMessage(hEditIn,
					  WM_SETTEXT,
					  NULL,
					  (LPARAM)"Waiting for client to connect...");
			PostMessage(hEditIn, EM_SETREADONLY, 1, 0);

			//create send label
			CreateWindowEx(WS_EX_CLIENTEDGE,
				"STATIC",
				"Send, Context Send To Client",
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				50,
				270,
				400,
				20,
				hWnd,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			// Create outgoing message box
			hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE,
						"EDIT",
						"",
						WS_CHILD | WS_VISIBLE | ES_MULTILINE |
						ES_AUTOVSCROLL | ES_AUTOHSCROLL,
						50,
						300,
						400,
						60,
						hWnd,
						(HMENU)IDC_EDIT_IN,
						GetModuleHandle(NULL),
						NULL);
			if (!hEditOut)
			{
				MessageBox(hWnd,
						"Could not create outgoing edit box.",
						"Error",
						MB_OK | MB_ICONERROR);
			}

			SendMessage(hEditOut,
						WM_SETFONT,
						(WPARAM)hfDefault,
						MAKELPARAM(FALSE, 0));
			SendMessage(hEditOut,
					  WM_SETTEXT,
					  NULL,
					  (LPARAM)"Type message here...");

			// Create a push button
			HWND hWndButton = CreateWindow(
						"BUTTON",
						"Send",
						WS_TABSTOP | WS_VISIBLE |
						WS_CHILD | BS_DEFPUSHBUTTON,
						50,
						380,
						75,
						23,
						hWnd,
						(HMENU)IDC_MAIN_BUTTON,
						GetModuleHandle(NULL),
						NULL);

			SendMessage(hWndButton,
					  WM_SETFONT,
					  (WPARAM)hfDefault,
					  MAKELPARAM(FALSE, 0));

			WSADATA WsaDat;
			int nResult = WSAStartup(MAKEWORD(2, 2), &WsaDat);
			if (nResult != 0)
			{
				MessageBox(hWnd,
					  "Winsock initialization failed",
					  "Critical Error",
					  MB_ICONERROR);
					  SendMessage(hWnd, WM_DESTROY, NULL, NULL);
			    break;
			}

			Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (Socket == INVALID_SOCKET)
			{
				MessageBox(hWnd,
						"Socket creation failed",
						"Critical Error",
						MB_ICONERROR);
						SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				break;
			}

			SOCKADDR_IN SockAddr;
			SockAddr.sin_port = htons(nPort);
			SockAddr.sin_family = AF_INET;
			SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(Socket, (LPSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
			{
				MessageBox(hWnd, "Unable to bind socket", "Error", MB_OK);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				break;
			}

			nResult = WSAAsyncSelect(Socket,
						  hWnd,
						  WM_SOCKET,
						  (FD_CLOSE | FD_ACCEPT | FD_READ));
			if (nResult)
			{
				MessageBox(hWnd,
				"WSAAsyncSelect failed",
				"Critical Error",
				MB_ICONERROR);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				break;
			}

			if (listen(Socket, (1)) == SOCKET_ERROR)
			{
				MessageBox(hWnd,
						"Unable to listen!",
						"Error",
						MB_OK);
				SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				break;
			}
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDC_MAIN_BUTTON:
			{
				char szBuffer[1024];
				ZeroMemory(szBuffer, sizeof(szBuffer));

				SendMessage(hEditOut,
							WM_GETTEXT,
							sizeof(szBuffer),
							reinterpret_cast<LPARAM>(szBuffer));

				send(Socket, szBuffer, strlen(szBuffer), 0);

				SendMessage(hEditOut, WM_SETTEXT, NULL, (LPARAM)"");
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		shutdown(Socket, SD_BOTH);
		closesocket(Socket);
		WSACleanup();
		break;
	case WM_SOCKET:
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
				{
					char szIncoming[1024];
					ZeroMemory(szIncoming, sizeof(szIncoming));

					int inDataLength = recv(Socket,
										  (char*)szIncoming,
										  sizeof(szIncoming) / sizeof(szIncoming[0]),
										  0);

					strncat_s(szHistory, szIncoming, inDataLength);
					strcat_s(szHistory, "\r\n");

					SendMessage(hEditIn,
								WM_SETTEXT,
								sizeof(szIncoming)-1,
								reinterpret_cast<LPARAM>(&szHistory));
				}
				break;

			case FD_CLOSE:
				{
					MessageBox(hWnd,
								"Client closed connection",
								"Connection closed!",
								MB_ICONINFORMATION | MB_OK);
					closesocket(Socket);
					SendMessage(hWnd, WM_DESTROY, NULL, NULL);
				}
				break;

			case FD_ACCEPT:
				{
					int size = sizeof(sockaddr);
					Socket = accept(wParam, &sockAddrClient, &size);
					if (Socket == INVALID_SOCKET)
					{
						int nret = WSAGetLastError();
						WSACleanup();
					}
					SendMessage(hEditIn,
								WM_SETTEXT,
								NULL,
								(LPARAM)"Client connected!");
				}
				break;
			}
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
