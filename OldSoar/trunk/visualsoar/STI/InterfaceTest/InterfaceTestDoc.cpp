// InterfaceTestDoc.cpp : implementation of the CTInterfaceTestDoc class
//

#include "stdafx.h"
#include "InterfaceTest.h"
#include "..\STI_Interface\STI_Runtime.h"
#include "..\STI_Interface\STI_Tool.h"
#include "..\STI_Interface\STI_CommonAPI.h"
#include "..\STI_Interface\STI_CommandIDs.h"

#pragma comment(lib, "..\\STI_Interface\\libSTI1.lib")

#include "InterfaceTestDoc.h"
#include "InterfaceTestView.h"

#undef UNUSED
#define UNUSED(x) (void)(x)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestDoc

IMPLEMENT_DYNCREATE(CTInterfaceTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CTInterfaceTestDoc, CDocument)
	//{{AFX_MSG_MAP(CTInterfaceTestDoc)
	ON_COMMAND(IDM_RUNTIME_START, OnRuntimeStart)
	ON_COMMAND(IDM_RUNTIME_STOP, OnRuntimeStop)
	ON_COMMAND(IDM_RUNTIME_EDIT_PROD, OnRuntimeEditProd)
	ON_UPDATE_COMMAND_UI(IDM_RUNTIME_STOP, OnUpdateRuntime)
	ON_COMMAND(IDM_TOOL_START, OnToolStart)
	ON_COMMAND(IDM_TOOL_STOP, OnToolStop)
	ON_COMMAND(IDM_TOOL_SEND_PRODUCTION, OnToolSendProduction)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_STOP, OnUpdateTool)
	ON_UPDATE_COMMAND_UI(IDM_RUNTIME_START, OnUpdateRuntimeStart)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_START, OnUpdateToolStart)
	ON_COMMAND(IDM_ACTIVE_ALL, OnActiveAll)
	ON_COMMAND(IDM_ACTIVE_NONE, OnActiveNone)
	ON_COMMAND(IDM_TOOL_EXCISE_PRODUCTION, OnToolExciseProduction)
	ON_COMMAND(IDM_TOOL_PRODUCTION_MATCHES, OnToolProductionMatches)
	ON_COMMAND(IDM_TOOL_RAW_COMMAND, OnToolRawCommand)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_SEND_PRODUCTION, OnUpdateTool)
	ON_UPDATE_COMMAND_UI(IDM_RUNTIME_EDIT_PROD, OnUpdateRuntime)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_EXCISE_PRODUCTION, OnUpdateTool)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_PRODUCTION_MATCHES, OnUpdateTool)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_RAW_COMMAND, OnUpdateTool)
	ON_COMMAND(IDM_TOOL_SEND_FILE, OnToolSendFile)
	ON_UPDATE_COMMAND_UI(IDM_TOOL_SEND_FILE, OnUpdateTool)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDM_NAME_RUNTIME1, IDM_NAME_TOOL10, OnName)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NAME_RUNTIME1, IDM_NAME_TOOL10, OnUpdateName)
	ON_COMMAND_RANGE(IDM_ACTIVE_ITEM, IDM_ACTIVE_ITEM+20, OnActive)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ACTIVE_ITEM, IDM_ACTIVE_ITEM+20, OnUpdateActive)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestDoc construction/destruction

CTInterfaceTestDoc::CTInterfaceTestDoc()
{
	// TODO: add one-time construction code here
	m_bIsRuntime = false ;
	m_bIsTool	 = false ;
	m_pView		 = NULL ;
	m_nNameID	 = 1 ;		// Maps to runtime 1
	m_hServer	 = 0 ;
}

CTInterfaceTestDoc::~CTInterfaceTestDoc()
{
}

BOOL CTInterfaceTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	// Don't prompt to save file on close--we don't usually want to.
	SetModifiedFlag(FALSE) ;

	return TRUE;
}

CTInterfaceTestView* CTInterfaceTestDoc::GetView()
{
	// If we've been called before, return the cached result.
	// Otherwise, find the first (and only) view associated with this doc.
	if (!m_pView)
		m_pView = (CTInterfaceTestView*)(m_viewList.GetHead()) ;

	return m_pView ;
}

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestDoc serialization

void CTInterfaceTestDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestDoc diagnostics

#ifdef _DEBUG
void CTInterfaceTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTInterfaceTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestDoc commands

void CTInterfaceTestDoc::OnRuntimeStart() 
{
	CString name = GetInstanceName() ;
	SetTitleAndPath(name) ;

	// Make sure this window is active so the debug trace will hook up correctly.
	AfxGetMainWnd()->SetActiveWindow() ;

	// Initialize the interface library and the listening port
	m_hServer = STI_InitInterfaceLibrary(name, true /* Is Runtime */) ;

	bool ok = m_hServer && STI_InitListenPort(m_hServer) ;

	if (ok)
	{
		m_bIsRuntime = true ;

		STI_EstablishConnections(m_hServer, NULL /* IP */, true /* Stop on first not found */) ;

		GetView()->StartMessagePumpTimer() ;
	}
}

void CTInterfaceTestDoc::OnRuntimeStop() 
{
	// TODO: Add your command handler code here
	m_bIsRuntime = false ;

	GetView()->StopMessagePumpTimer() ;

	STI_TerminateInterfaceLibrary(m_hServer) ;
	m_hServer = NULL ;
}

void CTInterfaceTestDoc::OnRuntimeEditProd() 
{
	// TODO: Add your command handler code here
	::STI_EditProduction(m_hServer, "EditProd1") ;
}

void CTInterfaceTestDoc::OnUpdateRuntime(CCmdUI* pCmdUI) 
{
	// Runtime commands only available after taking on the runtime role.
	pCmdUI->Enable(m_bIsRuntime) ;	
}

CString CTInterfaceTestDoc::GetInstanceName()
{
	// long	m_nNameID ;		// 1-10 => Runtime1-10 ; 11-20 => Tool1-10

	CString format = m_nNameID > 10 ? "Tool%d" : "Runtime%d" ;

	CString name ;
	name.Format(format, ((m_nNameID-1) % 10) + 1) ;

	return name ;
}

// Override DoFileSave, so we don't prompt with a dialog
// the first time this is called (and no file exists).
BOOL CTInterfaceTestDoc::DoFileSave()
{
	return DoSave(m_strPathName) ;
}

void CTInterfaceTestDoc::SetTitleAndPath(TCHAR const* pName)
{
	// Set the window name and the default path name for saves
	// so can send window message to just "save" the file w/o having
	// to provide a name.

	// Get the app path
	TCHAR path[_MAX_PATH*2] ;
	HINSTANCE hInst = ::GetModuleHandle(NULL) ;
	::GetModuleFileName(hInst, path, sizeof(path)) ;

	// Remove the filename and leave just the folder name
	TCHAR* pFolder = strrchr(path, '\\') + 1 ;
	*pFolder = NULL ;

	// Add the name to the path
	lstrcat(path, pName) ;
	lstrcat(path, _T(".txt")) ;
	SetPathName(path, FALSE) ;

	// Set the window title as well.
	SetTitle(pName) ;
}

void CTInterfaceTestDoc::OnToolStart() 
{
	// If the name field is still set to its default (Runtime1) then
	// override it and make it Tool1 if we start up a tool.
	if (m_nNameID == 1)
		m_nNameID = 11 ;
	
	CString name = GetInstanceName() ;
	SetTitleAndPath(name) ;

	// Make sure this window is active so the debug trace will hook up correctly.
	AfxGetMainWnd()->SetActiveWindow() ;

	m_hServer = STI_InitInterfaceLibrary(name, false /* Is Runtime */) ;
	bool ok = m_hServer && STI_InitListenPort(m_hServer);
	
	if (ok)
	{
		m_bIsTool = true ;

		STI_EstablishConnections(m_hServer, NULL, true /* Stop on first not found */) ;

		GetView()->StartMessagePumpTimer() ;
	}
}

void CTInterfaceTestDoc::OnToolStop() 
{
	// TODO: Add your command handler code here
	m_bIsTool = false ;

	GetView()->StopMessagePumpTimer() ;

	STI_TerminateInterfaceLibrary(m_hServer) ;
	m_hServer = NULL ;
}

void CTInterfaceTestDoc::OnToolSendProduction() 
{
	// TODO: Add your command handler code here
	::STI_SendProduction(m_hServer, "Production1", "Body1\nLine2\nLine3 which is a long long line and the total buffer size will end up being quite large\nLine 4 is long too.....hello there world.  This is more and more of this production body\nBody continues on to here too.  This is even more.\nOK now the end comes.\n") ;
}

void CTInterfaceTestDoc::OnUpdateTool(CCmdUI* pCmdUI) 
{
	// Tool commands only available after taking on the "tool" role
	pCmdUI->Enable(m_bIsTool) ;
}

void CTInterfaceTestDoc::OnUpdateRuntimeStart(CCmdUI* pCmdUI) 
{
	// Can only start when not currently in either role
	pCmdUI->Enable(!m_bIsTool && !m_bIsRuntime) ;
}

void CTInterfaceTestDoc::OnUpdateToolStart(CCmdUI* pCmdUI) 
{
	// Can only start when not currently in either role
	pCmdUI->Enable(!m_bIsRuntime && !m_bIsTool) ;	
}

void CTInterfaceTestDoc::ProcessCommand(long commandID, long commandFlags, long dataSize, long systemMsg,
										long params[6], CString& stringParam, char* pData)
{
	UNUSED(systemMsg) ;
	UNUSED(commandFlags) ;

	// BUGBUG: This might drop data from pData if there are embedded nulls,
	// but good enough for quick showing of the results.
	CString dataPart = pData ? pData : "--No data-" ;

	CString output ;
	output.Format("Incoming command is %d string param <%s> data size %d data <%s>\r\n",
		commandID, stringParam, dataSize, dataPart) ;

	//GetView()->ShowString(output) ;

	switch (commandID)
	{
	case STI_kSystemNameCommand:
		{
			// When a name command comes in we add the new named port to the menu of connections
			long port = params[0] ;
			CString name = stringParam ;

			GetView()->AddConnection(name, port) ;
			break ;
		}
	}
}

bool CTInterfaceTestDoc::PumpMessages()
{
	// Send and receive any pending messages
	bool ok = ::STI_PumpMessages(m_hServer, true) ;

	// Check to see if any commands arrived
	while (::STI_IsIncomingCommandAvailable(m_hServer))
	{
		GetView()->ShowString("Got incoming command\r\n") ;

		long commandID, commandFlags, dataSize, systemMsg ;
		long params[6] ;
		::STI_GetIncomingCommand1(m_hServer, &commandID, &commandFlags, &dataSize, &systemMsg, &params[0],
								  &params[1], &params[2], &params[3], &params[4], &params[5]);

		CString stringParam = ::STI_GetIncomingCommandStringParam1(m_hServer) ;

		// Use memcpy to get the data field
		char* pData = NULL ;
		if (dataSize > 0)
		{
			pData = new char[dataSize] ;
			memcpy(pData, ::STI_GetIncomingCommandData(m_hServer), dataSize) ;
		}

		ProcessCommand(commandID, commandFlags, dataSize, systemMsg, params,
					   stringParam, pData) ;

		::STI_PopIncomingCommand(m_hServer) ;
	}

	return ok ;
}

void CTInterfaceTestDoc::OnName(UINT nID) 
{
	// long	m_nNameID ;		// 1-10 => Runtime 1-10 ; 11-20 => Tool 1-10
	m_nNameID = nID - IDM_NAME_RUNTIME1 + 1;
}

void CTInterfaceTestDoc::OnUpdateName(CCmdUI* pCmdUI) 
{
	// Set check next to current name choice
	UINT nID = m_nNameID + IDM_NAME_RUNTIME1 - 1 ;
	pCmdUI->SetCheck(pCmdUI->m_nID == nID) ;
}

void CTInterfaceTestDoc::OnActiveAll() 
{
	STI_EnableAllConnections(m_hServer, true) ;	
}

void CTInterfaceTestDoc::OnActiveNone() 
{
	STI_EnableAllConnections(m_hServer, false) ;
}

void CTInterfaceTestDoc::OnActive(UINT nID)
{
	long index = nID - IDM_ACTIVE_ITEM ;

	bool isEnabled = STI_IsConnectionEnabledByIndex(m_hServer, index) ;

	STI_EnableConnectionByIndex(m_hServer, index, !isEnabled) ;
}

void CTInterfaceTestDoc::OnUpdateActive(CCmdUI* pCmdUI)
{
	long index = pCmdUI->m_nID - IDM_ACTIVE_ITEM ;

	bool isEnabled = STI_IsConnectionEnabledByIndex(m_hServer, index) ;

	pCmdUI->SetCheck(isEnabled) ;
}

void CTInterfaceTestDoc::OnToolExciseProduction() 
{
	::STI_ExciseProduction(m_hServer, "Production1") ;
}

void CTInterfaceTestDoc::OnToolProductionMatches() 
{
	::STI_ProductionMatches(m_hServer, "Production1") ;	
}

void CTInterfaceTestDoc::OnToolRawCommand() 
{
	::STI_ProductionMatches(m_hServer, "This is a raw command string with some embedded\nnewline chars\n\n.") ;		
}

void CTInterfaceTestDoc::OnToolSendFile() 
{
	// BUGBUG: Need to decide which file system we're using here--Windows or Unix.
	::STI_ExciseProduction(m_hServer, "c:\\file.soar") ;	
}
