// MainFrame.cpp : implementation of the CTMainFrame class
//

#include "stdafx.h"
#include "InterfaceTest.h"
#include "MainFrame.h"
#include "InterfaceTestView.h"
#include "../SoarSocket/Check.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMainFrame

IMPLEMENT_DYNCREATE(CTMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CTMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CTMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER+1, OnShowString)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CTMainFrame construction/destruction

CTMainFrame::CTMainFrame()
{
	// TODO: add member initialization code here
	
}

CTMainFrame::~CTMainFrame()
{
}

int CTMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

// Taken from AfxRegisterWndClass().
static BOOL RegisterClassName(TCHAR const* lpszName, UINT nClassStyle = NULL,
							  HCURSOR hCursor = 0, HBRUSH hbrBackground = 0, HICON hIcon = 0)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	// see if the class already exists
	WNDCLASS wndcls;
	if (::GetClassInfo(hInst, lpszName, &wndcls))
	{
		// already registered, assert everything is good
		ASSERT(wndcls.style == nClassStyle);

		// NOTE: We have to trust that the hIcon, hbrBackground, and the
		//  hCursor are semantically the same, because sometimes Windows does
		//  some internal translation or copying of those handles before
		//  storing them in the internal WNDCLASS retrieved by GetClassInfo.
		return TRUE ;
	}

	// otherwise we need to register a new class
	wndcls.style = nClassStyle;
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hInstance = hInst;
	wndcls.hIcon = hIcon;
	wndcls.hCursor = hCursor;
	wndcls.hbrBackground = hbrBackground;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = lpszName;

	return AfxRegisterClass(&wndcls) ;
}

BOOL CTMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.x = 75 ;
	cs.y = 30 ;

	// Make the window skiny so 2 fit side by side
	// BADBAD: Better to save this in the registry and restore here.
	cs.cx = 400 ;
	cs.cy = 700 ;

	TCHAR const* pName = "STI_TestClass" ;

	VERIFY(RegisterClassName(pName)) ;
	cs.lpszClass = pName ;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTMainFrame diagnostics

#ifdef _DEBUG
void CTMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CTMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTMainFrame message handlers

// Provides method to call ShowString using a windows message.
// PrintDebug uses this.
// The wParam contains a pointer, so this MUST be called
// using SendMessage or the pointer could become invalid before
// the message is received here.  Also MUST be called from
// within the same process (to go cross process we'd need to
// use WM_COPYDATA instead for this).
LRESULT CTMainFrame::OnShowString(WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam) ;

	TCHAR const* pStr = (TCHAR const*)wParam ;
	
	CTInterfaceTestView* pView = (CTInterfaceTestView*)this->GetActiveView() ;

	if (pView)
		pView->ShowString(pStr) ;

	return 0 ;
}
