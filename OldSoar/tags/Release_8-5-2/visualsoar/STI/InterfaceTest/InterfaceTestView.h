// InterfaceTestView.h : interface of the CTInterfaceTestView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACETESTVIEW_H__F72B6627_3A32_4EB4_992C_6406D2997A5F__INCLUDED_)
#define AFX_INTERFACETESTVIEW_H__F72B6627_3A32_4EB4_992C_6406D2997A5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const long kPumpMessageTimer = 1 ;
const long kPumpMessageDelay = 1000 ;

class CTInterfaceTestDoc ;

class CTInterfaceTestView : public CEditView
{
protected: // create from serialization only
	CTInterfaceTestView();
	DECLARE_DYNCREATE(CTInterfaceTestView)

// Attributes
public:
	CTInterfaceTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTInterfaceTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ShowString(TCHAR const* pString);
	bool StopMessagePumpTimer();
	bool StartMessagePumpTimer();
	virtual ~CTInterfaceTestView();

	void AddConnection(TCHAR const* pName, long port) ;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	UINT m_PumpSpeedID ;
	bool m_Pumping ;

	void SetPumpSpeed(UINT nID) ;

// Generated message map functions
protected:
	//{{AFX_MSG(CTInterfaceTestView)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPumpSpeed100();
	afx_msg void OnUpdatePumpSpeed(CCmdUI* pCmdUI);
	afx_msg void OnPumpSpeed10();
	afx_msg void OnPumpSpeed1();
	afx_msg void OnPumpSpeed10s();
	afx_msg void OnPumpSpeed2();
	afx_msg void OnPumpSpeed30s();
	afx_msg void OnPumpSpeed5s();
	afx_msg void OnPumpSpeed60s();
	//}}AFX_MSG
	BOOL OnEditChange() ;

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in InterfaceTestView.cpp
inline CTInterfaceTestDoc* CTInterfaceTestView::GetDocument()
   { return (CTInterfaceTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERFACETESTVIEW_H__F72B6627_3A32_4EB4_992C_6406D2997A5F__INCLUDED_)
