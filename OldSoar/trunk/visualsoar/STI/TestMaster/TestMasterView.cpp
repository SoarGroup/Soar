// TestMasterView.cpp : implementation of the CTestMasterView class
//

#include "stdafx.h"
#include "TestMaster.h"

#include "TestMasterDoc.h"
#include "TestMasterView.h"
#include "Results1.h"

#include "../InterfaceTest/resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView

IMPLEMENT_DYNCREATE(CTestMasterView, CEditView)

BEGIN_MESSAGE_MAP(CTestMasterView, CEditView)
	//{{AFX_MSG_MAP(CTestMasterView)
	ON_COMMAND(IDM_TEST_TEST1, OnTestTest1)
	ON_COMMAND(IDM_TEST_TEST2, OnTestTest2)
	ON_COMMAND(IDM_TEST_ALL, OnTestAll)
	ON_COMMAND(IDM_TEST_TEST3, OnTestTest3)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView construction/destruction

CTestMasterView::CTestMasterView()
{
	// TODO: add construction code here

}

CTestMasterView::~CTestMasterView()
{
}

BOOL CTestMasterView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView drawing

void CTestMasterView::OnDraw(CDC* pDC)
{
	ASSERT(pDC) ;

	CTestMasterDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView printing

BOOL CTestMasterView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CTestMasterView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing.
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CTestMasterView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView diagnostics

#ifdef _DEBUG
void CTestMasterView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTestMasterView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CTestMasterDoc* CTestMasterView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTestMasterDoc)));
	return (CTestMasterDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestMasterView message handlers

static bool StartTests(long number)
{
	for (int i = 0 ; i < number ; i++)
		WinExec("InterfaceTest.exe", SW_SHOW) ;

	// Make sure everything gets started (this probably isn't necessary
	// but we don't want any timing problems).
	Sleep(100) ;

	return true ;
}

static const long kMaxTestApps = 50 ;

BOOL CALLBACK EnumWindowsProc(
  HWND hWnd,      // handle to parent window
  LPARAM lParam   // application-defined value
)
{
	TCHAR name[256] ;
	::GetWindowText(hWnd, name, sizeof(name)) ;

	TCHAR className[256] ;
	::GetClassName(hWnd, className, sizeof(className)) ;

	HWND* pTests = (HWND*) lParam ;

	// See if the window's class matches what we're looking for.
	if (lstrcmp("STI_TestClass", className) == 0)
	{
		OutputDebugString(name) ;
		OutputDebugString("\n") ;

		// Find an empty slot in the tests array and put this new window there.
		for (int i = 0 ; i < kMaxTestApps ; i++)
			if (pTests[i] == NULL)
			{
				pTests[i] = hWnd ;
				return TRUE ;
			}
	}

	// Keep searching through windows.
	return TRUE ;
}

static BOOL FindTestWindows(HWND tests[], long number)
{
	for (int i = 0 ; i < kMaxTestApps ; i++)
		tests[i] = NULL ;

	LPARAM lParam = (LPARAM)tests ;

	// Enumerate all top level windows and find the one's we just started
	EnumWindows(EnumWindowsProc, lParam) ;

	// Check that we found the expected number of windows.
	long count = 0 ;
	for (i = 0 ; i < kMaxTestApps ; i++)
		if (tests[i] != NULL)
			count++ ;

	return (count == number) ;
}

static BOOL CloseTests(HWND tests[], long number)
{
	for (int i = 0 ; i < number ; i++)
	{
		::SendMessage(tests[i], WM_CLOSE, 0, 0) ;
	}

	return TRUE ;
}

BOOL RenameLogFile(TCHAR const* pSrc, TCHAR const* pDest)
{
	TCHAR srcPath[_MAX_PATH*2] ;
	GetFilePath(srcPath, sizeof(srcPath), pSrc) ;

	TCHAR destPath[_MAX_PATH*2] ;
	GetFilePath(destPath, sizeof(destPath), pDest) ;

	// Delete the dest file (if there's already one)
	::DeleteFile(destPath) ;

	// Then do the move
	BOOL ok = ::MoveFile(srcPath, destPath) ;

	ASSERT(ok) ;

	return ok ;
}

// Test1: Simple connect and disconnect test.
// Test2: Connect and send production to runtimes, edit production to tool
// Test3: Connect and send 50 productions, edit 50 productions.
static void RunTest_2Runtimes_1Tool(long testID)
{
	HWND tests[kMaxTestApps] ;

	long kNumberTests = 3 ;

	// Start the test apps running
	StartTests(kNumberTests) ;
	
	// Locate the main frames of the apps we just started
	BOOL bFound = FindTestWindows(tests, kNumberTests) ;

	if (!bFound)
		return ;

	// Set the names up, two runtimes and one tool
	::SendMessage(tests[0], WM_COMMAND, IDM_NAME_RUNTIME1, 0) ;
	::SendMessage(tests[1], WM_COMMAND, IDM_NAME_RUNTIME2, 0) ;
	::SendMessage(tests[2], WM_COMMAND, IDM_NAME_TOOL1, 0) ;

	// Start both runtimes, then the tool--all should connect.
	::SendMessage(tests[0], WM_COMMAND, IDM_RUNTIME_START, 0) ;

	Sleep(1000) ;
	::SendMessage(tests[1], WM_COMMAND, IDM_RUNTIME_START, 0) ;

	Sleep(1000) ;
	::SendMessage(tests[2], WM_COMMAND, IDM_TOOL_START, 0) ;
	
	// Wait for the connections to all occur
	Sleep(5000) ;

	// Tests 2 and 3 send messages to and from
	if (testID == 2 || testID == 3)
	{
		long count = (testID == 2) ? 1 : 50 ;

		// Set the pump speed to 1/10 sec
		::SendMessage(tests[0], WM_COMMAND, IDM_PUMP_SPEED_10, 0) ;
		::SendMessage(tests[1], WM_COMMAND, IDM_PUMP_SPEED_10, 0) ;
		::SendMessage(tests[2], WM_COMMAND, IDM_PUMP_SPEED_10, 0) ;

		for (int i = 0 ; i < count ; i++)
		{
			// Send production from tool to the runtimes.
			::SendMessage(tests[2], WM_COMMAND, IDM_TOOL_SEND_PRODUCTION, 0) ;
			Sleep(1000) ;
			
			// Send edit production from each runtime to the tools
			::SendMessage(tests[0], WM_COMMAND, IDM_RUNTIME_EDIT_PROD, 0) ;
			Sleep(500) ;

			::SendMessage(tests[1], WM_COMMAND, IDM_RUNTIME_EDIT_PROD, 0) ;
			Sleep(1000) ;
		}

		// Make sure all messages have been consumed
		Sleep(2000) ;
	}

	// Then close down all of the connections
	::SendMessage(tests[0], WM_COMMAND, IDM_RUNTIME_STOP, 0) ;
	::SendMessage(tests[1], WM_COMMAND, IDM_RUNTIME_STOP, 0) ;
	::SendMessage(tests[2], WM_COMMAND, IDM_TOOL_STOP, 0) ;

	// Wait for the shutdowns to complete
	Sleep(1000) ;

	// Save the logs
	::SendMessage(tests[0], WM_COMMAND, ID_FILE_SAVE, 0) ;
	::SendMessage(tests[1], WM_COMMAND, ID_FILE_SAVE, 0) ;
	::SendMessage(tests[2], WM_COMMAND, ID_FILE_SAVE, 0) ;

	// Rename the logs
	TCHAR output1[_MAX_PATH], output2[_MAX_PATH], output3[_MAX_PATH] ;
	wsprintf(output1, "Test%d_Runtime1.txt",testID) ;
	wsprintf(output2, "Test%d_Runtime2.txt",testID) ;
	wsprintf(output3, "Test%d_Tool1.txt",testID) ;

	RenameLogFile("Runtime1.txt", output1) ;
	RenameLogFile("Runtime2.txt", output2) ;
	RenameLogFile("Tool1.txt", output3) ;

	// Close down all of the test apps
	CloseTests(tests, kNumberTests) ;
}

void CTestMasterView::OnTestTest1() 
{
	RunTest_2Runtimes_1Tool(1) ;

	// Display the results
	CTResults1 results ;
	results.DoModal() ;
}

void CTestMasterView::OnTestTest2() 
{
	RunTest_2Runtimes_1Tool(2) ;

	// Display the results
	CTResults1 results ;
	results.DoModal() ;	
}

void CTestMasterView::OnTestTest3() 
{
	RunTest_2Runtimes_1Tool(3) ;

	// Display the results
	CTResults1 results ;
	results.DoModal() ;		
}

void CTestMasterView::OnTestAll() 
{
	RunTest_2Runtimes_1Tool(1) ;
	RunTest_2Runtimes_1Tool(2) ;
	RunTest_2Runtimes_1Tool(3) ;

	// Display the results
	CTResults1 results ;
	results.DoModal() ;
}
