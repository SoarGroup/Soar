// TestMasterDoc.cpp : implementation of the CTestMasterDoc class
//

#include "stdafx.h"
#include "TestMaster.h"

#include "TestMasterDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestMasterDoc

IMPLEMENT_DYNCREATE(CTestMasterDoc, CDocument)

BEGIN_MESSAGE_MAP(CTestMasterDoc, CDocument)
	//{{AFX_MSG_MAP(CTestMasterDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestMasterDoc construction/destruction

CTestMasterDoc::CTestMasterDoc()
{
	// TODO: add one-time construction code here

}

CTestMasterDoc::~CTestMasterDoc()
{
}

BOOL CTestMasterDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTestMasterDoc serialization

void CTestMasterDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CTestMasterDoc diagnostics

#ifdef _DEBUG
void CTestMasterDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTestMasterDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestMasterDoc commands
