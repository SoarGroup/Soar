// InterfaceTestView.cpp : implementation of the CTInterfaceTestView class
//

#include "stdafx.h"
#include "InterfaceTest.h"

#include "InterfaceTestDoc.h"
#include "InterfaceTestView.h"
#include "resource.h"
#include "../SoarSocket/Check.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView

IMPLEMENT_DYNCREATE(CTInterfaceTestView, CEditView)

BEGIN_MESSAGE_MAP(CTInterfaceTestView, CEditView)
	//{{AFX_MSG_MAP(CTInterfaceTestView)
	ON_WM_TIMER()
	ON_COMMAND(IDM_PUMP_SPEED_100, OnPumpSpeed100)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_100, OnUpdatePumpSpeed)
	ON_COMMAND(IDM_PUMP_SPEED_10, OnPumpSpeed10)
	ON_COMMAND(IDM_PUMP_SPEED_1, OnPumpSpeed1)
	ON_COMMAND(IDM_PUMP_SPEED_10S, OnPumpSpeed10s)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_10, OnUpdatePumpSpeed)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_1, OnUpdatePumpSpeed)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_10S, OnUpdatePumpSpeed)
	ON_COMMAND(IDM_PUMP_SPEED_2, OnPumpSpeed2)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_2, OnUpdatePumpSpeed)
	ON_COMMAND(IDM_PUMP_SPEED_30S, OnPumpSpeed30s)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_30S, OnUpdatePumpSpeed)
	ON_COMMAND(IDM_PUMP_SPEED_5S, OnPumpSpeed5s)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_5S, OnUpdatePumpSpeed)
	ON_COMMAND(IDM_PUMP_SPEED_60S, OnPumpSpeed60s)
	ON_UPDATE_COMMAND_UI(IDM_PUMP_SPEED_60S, OnUpdatePumpSpeed)
	//}}AFX_MSG_MAP
	ON_CONTROL_REFLECT_EX(EN_CHANGE, OnEditChange)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView construction/destruction

CTInterfaceTestView::CTInterfaceTestView()
{
	// TODO: add construction code here
	m_PumpSpeedID = IDM_PUMP_SPEED_1 ;
	m_Pumping     = false ;
}

CTInterfaceTestView::~CTInterfaceTestView()
{
}

BOOL CTInterfaceTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping
	cs.style |= ES_READONLY ; // This is an output window

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView drawing

void CTInterfaceTestView::OnDraw(CDC* pDC)
{
	UNUSED(pDC) ;

	CTInterfaceTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView printing

BOOL CTInterfaceTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CTInterfaceTestView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing.
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CTInterfaceTestView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView diagnostics

#ifdef _DEBUG
void CTInterfaceTestView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTInterfaceTestView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CTInterfaceTestDoc* CTInterfaceTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTInterfaceTestDoc)));
	return (CTInterfaceTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestView message handlers

BOOL CTInterfaceTestView::OnEditChange()
{
	// This is the code in CEditView::OnEditChange().
	//ASSERT_VALID(this);
	//GetDocument()->SetModifiedFlag();
	//ASSERT_VALID(this) ;
	
	// We don't want to prompt for saving on close since the user
	// won't normally want to do that...so don't want to keep resetting
	// the modified flag to TRUE.  So we intercept this message
	// and do nothing.

	return TRUE ;
}

void CTInterfaceTestView::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent)
	{
	case kPumpMessageTimer:
		GetDocument()->PumpMessages() ; break ;
	default:
		ASSERT(FALSE) ; break ; // Unrecognized timer message
	}
	
	CEditView::OnTimer(nIDEvent);
}

bool CTInterfaceTestView::StartMessagePumpTimer()
{
	ShowString("Starting timer\r\n") ;

	m_Pumping = true ;

	SetPumpSpeed(m_PumpSpeedID) ;

	return true ;
}

bool CTInterfaceTestView::StopMessagePumpTimer()
{
	m_Pumping = false ;

	UINT res = KillTimer(kPumpMessageTimer) ;

	ShowString("Stopped timer\r\n") ;

	return (res != 0) ;
}

void CTInterfaceTestView::ShowString(const TCHAR *pString)
{	
	// There should be no selection, so this inserts the text.
	GetEditCtrl().ReplaceSel(pString) ;
}

void CTInterfaceTestView::SetPumpSpeed(UINT nID)
{
	m_PumpSpeedID = nID ;

	// If we're not pumping messages at the moment
	// no need to actually set the timer to the new
	// value.
	if (!m_Pumping)
		return ;

	long millisecs = 1000 ;

	// Convert from menu ID to millisecs of delay between
	// calls to pump messages.
	switch (nID)
	{
		case IDM_PUMP_SPEED_100: millisecs = 10 ; break ;
		case IDM_PUMP_SPEED_10:  millisecs = 100 ; break ;
		case IDM_PUMP_SPEED_2:   millisecs = 500 ; break ;
		case IDM_PUMP_SPEED_1:   millisecs = 1000 ; break ;
		case IDM_PUMP_SPEED_5S:  millisecs = 5000 ; break ;
		case IDM_PUMP_SPEED_10S: millisecs = 10000 ; break ;
		case IDM_PUMP_SPEED_30S: millisecs = 30000 ; break ;
		case IDM_PUMP_SPEED_60S: millisecs = 60000 ; break ;
		default: ASSERT(FALSE) ; millisecs = 1000 ; break ;
	}

	// Set the timer to the new value
	KillTimer(kPumpMessageTimer) ;
	SetTimer(kPumpMessageTimer, millisecs, NULL) ;
}

void CTInterfaceTestView::OnUpdatePumpSpeed(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(pCmdUI->m_nID == m_PumpSpeedID) ;
	pCmdUI->Enable(TRUE) ;
}

void CTInterfaceTestView::OnPumpSpeed100() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_100) ;
}

void CTInterfaceTestView::OnPumpSpeed10() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_10) ;
}

void CTInterfaceTestView::OnPumpSpeed1() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_1) ;
}

void CTInterfaceTestView::OnPumpSpeed10s() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_10S) ;	
}

void CTInterfaceTestView::OnPumpSpeed2() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_2) ;
}

void CTInterfaceTestView::OnPumpSpeed30s() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_30S) ;
}

void CTInterfaceTestView::OnPumpSpeed5s() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_5S) ;
}

void CTInterfaceTestView::OnPumpSpeed60s() 
{
	// TODO: Add your command handler code here
	SetPumpSpeed(IDM_PUMP_SPEED_60S) ;
}

void CTInterfaceTestView::AddConnection(TCHAR const* pName, long port)
{
	UNUSED(port) ;

	CMenu* pMenu = AfxGetMainWnd()->GetMenu() ;
	CMenu* pActiveMenu = pMenu->GetSubMenu(6) ;	// For test app just use hard-coded menu position (6)

	ASSERT(pActiveMenu) ;
	
	// How many items have been added (there are always 3 entries at least).
	long items = pActiveMenu->GetMenuItemCount() - 3 ;

	// Note: Not checking if this name is already in the list.
	// That's intentional for this test app--so we can see extra calls that
	// shouldn't be happening.  A production app might want to check.
	long newID = IDM_ACTIVE_ITEM + items ;
	pActiveMenu->InsertMenu((UINT)-1, MF_BYPOSITION | MF_STRING, newID, pName) ;
}
