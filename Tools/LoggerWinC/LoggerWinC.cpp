/////////////////////////////////////////////////////////////////
// Sample logging application for SML
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Feb 2006
//
// This application shows an example of how to build a simple
// logging tool.  To use it you would run a Soar kernel somewhere
// (in an environment or inside the debugger for example) and then
// run this logger.  The logger listens for certain events and
// then creates a log file.
//
// The idea is that you could take this simple app and modify it
// to log what you need for your specific application, outputing the
// data in whatever format you want.
//
// This sample is broken into two parts:
// LoggerWinC.cpp contains all of the Windows specific code for putting up a little
//                window to control the logging.
// Logger.cpp contains the SML code for actually doing the logging.
//
// This way if you're not using Windows or wish to add logging to an existing app
// the part you're interested in is Logger.cpp
//
/////////////////////////////////////////////////////////////////

#include "LoggerWinC.h"
#include "Logger.h"
#include <string>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

#define BUTTONCLASS "button"

// Logging filename to use as default
static std::string gFilename = "log.txt" ;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SetLogFilename(HWND, UINT, WPARAM, LPARAM);
HWND				hMainWnd ;

HWND hWndButton1, hWndButton2, hWndButton3, hWndButton4 ;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LOGGERWINC, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LOGGERWINC);

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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_LOGGERWINC);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_LOGGERWINC;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

void EnableItems()
{
	::EnableWindow(hWndButton1, !IsLogging()) ;	// Start
	::EnableWindow(hWndButton2, IsLogging()) ;	// Stop
	::EnableWindow(hWndButton3, !IsLogging()) ;	// Append
	::EnableWindow(hWndButton4, !IsLogging()) ; // Set path

	HMENU hMenu = ::GetMenu(hMainWnd) ;
	::EnableMenuItem(hMenu, ID_FILE_STARTLOGGING, !IsLogging() ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED) ;
	::EnableMenuItem(hMenu, ID_FILE_STOPLOGGING, IsLogging() ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED) ;
	::EnableMenuItem(hMenu, ID_FILE_STARTLOGGING32774, !IsLogging() ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED) ;
	::EnableMenuItem(hMenu, ID_FILE_SETLOGFILENAME, !IsLogging() ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED) ;
}

//
//   FUNCTION: InitInstance(HANDLE, int)
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

   int width = 350 ;
   int height = 150 ;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      300, 300, width, height, NULL, NULL, hInstance, NULL);
   hMainWnd = hWnd ;

   if (!hWnd)
   {
      return FALSE;
   }

	RECT rect ;
	::GetClientRect(hWnd, &rect) ;

	width = rect.right - rect.left ;
	height = rect.bottom - rect.top ;

	hWndButton1 = CreateWindow( BUTTONCLASS, "Start Logging", 
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                         0, 0, width/2, height/2,  hWnd,
                         (HMENU) ID_FILE_STARTLOGGING,
                         hInst, NULL );

	hWndButton2 = CreateWindow( BUTTONCLASS, "Stop Logging", 
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                         width/2, 0, width/2, height/2,  hWnd,
                         (HMENU) ID_FILE_STOPLOGGING,
                         hInst, NULL );

	hWndButton3 = CreateWindow( BUTTONCLASS, "Append Logging", 
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                         0, height/2, width/2, height/2,  hWnd,
                         (HMENU) ID_FILE_STARTLOGGING32774,
                         hInst, NULL );

	hWndButton4 = CreateWindow( BUTTONCLASS, "Set Log Filename", 
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                         width/2, height/2, width/2, height/2,  hWnd,
                         (HMENU) ID_FILE_SETLOGFILENAME,
                         hInst, NULL );

	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT) ;
	::SendMessage(hWndButton1, WM_SETFONT, (WPARAM)hFont, 1) ;
	::SendMessage(hWndButton2, WM_SETFONT, (WPARAM)hFont, 1) ;
	::SendMessage(hWndButton3, WM_SETFONT, (WPARAM)hFont, 1) ;
	::SendMessage(hWndButton4, WM_SETFONT, (WPARAM)hFont, 1) ;

	EnableItems() ;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
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
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_SETLOGFILENAME:
			DialogBox(hInst, (LPCTSTR)IDD_SETLOGFILENAME, hWnd, (DLGPROC)SetLogFilename) ;
			break ;
		case ID_FILE_STARTLOGGING:
		case ID_FILE_STARTLOGGING32774:
			{
				std::string errorMsg ;
				bool ok = StartLogging(gFilename.c_str(), (wmId != ID_FILE_STARTLOGGING), &errorMsg) ;
				if (!ok)
				{
					::MessageBox(hWnd, errorMsg.c_str(), "Error starting logging", MB_OK) ;
				}
				EnableItems() ;
				break ;
			}
		case ID_FILE_STOPLOGGING:
			{
				bool ok = StopLogging() ;
				EnableItems() ;
				break ;
			}
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			if (IsLogging())
				StopLogging() ;
			DestroyWindow(hWnd);
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
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK SetLogFilename(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		::SetWindowText(::GetDlgItem(hDlg, IDC_EDIT1), gFilename.c_str()) ;
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			TCHAR buffer[1024] ;
			::GetWindowText(::GetDlgItem(hDlg, IDC_EDIT1), buffer, sizeof(buffer)/sizeof(TCHAR)) ;
			gFilename = buffer ;

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
