// TestMasterView.h : interface of the CTestMasterView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTMASTERVIEW_H__D1ECEB82_4415_4665_ACA1_89706F6C0EB1__INCLUDED_)
#define AFX_TESTMASTERVIEW_H__D1ECEB82_4415_4665_ACA1_89706F6C0EB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTestMasterView : public CEditView
{
protected: // create from serialization only
	CTestMasterView();
	DECLARE_DYNCREATE(CTestMasterView)

// Attributes
public:
	CTestMasterDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestMasterView)
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
	virtual ~CTestMasterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestMasterView)
	afx_msg void OnTestTest1();
	afx_msg void OnTestTest2();
	afx_msg void OnTestAll();
	afx_msg void OnTestTest3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TestMasterView.cpp
inline CTestMasterDoc* CTestMasterView::GetDocument()
   { return (CTestMasterDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTMASTERVIEW_H__D1ECEB82_4415_4665_ACA1_89706F6C0EB1__INCLUDED_)
