// InterfaceTestDoc.h : interface of the CTInterfaceTestDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACETESTDOC_H__A5061B17_5C4A_4E4D_8135_00CFE6A83B66__INCLUDED_)
#define AFX_INTERFACETESTDOC_H__A5061B17_5C4A_4E4D_8135_00CFE6A83B66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\STI_Interface\STI_CommonAPI.h"	// For STI_Handle

// Forward declaration for the view.
class CTInterfaceTestView ;

class CTInterfaceTestDoc : public CDocument
{
protected: // create from serialization only
	CTInterfaceTestDoc();
	DECLARE_DYNCREATE(CTInterfaceTestDoc)

	bool		m_bIsRuntime ;
	bool		m_bIsTool ;
	long		m_nNameID ;		// 1-10 => Runtime 1-10 ; 11-20 => Tool 1-10
	STI_Handle	m_hServer ;

	CTInterfaceTestView* m_pView ;

	CString	GetInstanceName() ;

// Attributes
public:
	CTInterfaceTestView* GetView() ;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTInterfaceTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	bool PumpMessages();
	virtual ~CTInterfaceTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Override MFC's DoFileSave
	virtual BOOL DoFileSave() ;

protected:
	void SetTitleAndPath(TCHAR const* pName) ;

	void ProcessCommand(long commandID, long commandFlags, long dataSize, long systemMsg,
						long params[6], CString& stringParam, char* pData) ;

// Generated message map functions
protected:
	//{{AFX_MSG(CTInterfaceTestDoc)
	afx_msg void OnRuntimeStart();
	afx_msg void OnRuntimeStop();
	afx_msg void OnRuntimeEditProd();
	afx_msg void OnUpdateRuntime(CCmdUI* pCmdUI);
	afx_msg void OnToolStart();
	afx_msg void OnToolStop();
	afx_msg void OnToolSendProduction();
	afx_msg void OnUpdateTool(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRuntimeStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolStart(CCmdUI* pCmdUI);
	afx_msg void OnActiveAll();
	afx_msg void OnActiveNone();
	afx_msg void OnToolExciseProduction();
	afx_msg void OnToolProductionMatches();
	afx_msg void OnToolRawCommand();
	afx_msg void OnToolSendFile();
	//}}AFX_MSG
	void OnName(UINT nID);
	void OnUpdateName(CCmdUI* pCmdUI);
	void OnActive(UINT nID) ;
	void OnUpdateActive(CCmdUI* pCmdUI) ;
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERFACETESTDOC_H__A5061B17_5C4A_4E4D_8135_00CFE6A83B66__INCLUDED_)
