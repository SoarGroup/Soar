// TestMasterDoc.h : interface of the CTestMasterDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTMASTERDOC_H__02135E59_C2E6_4DB0_9BB3_CC231F93F938__INCLUDED_)
#define AFX_TESTMASTERDOC_H__02135E59_C2E6_4DB0_9BB3_CC231F93F938__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTestMasterDoc : public CDocument
{
protected: // create from serialization only
	CTestMasterDoc();
	DECLARE_DYNCREATE(CTestMasterDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestMasterDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestMasterDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestMasterDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTMASTERDOC_H__02135E59_C2E6_4DB0_9BB3_CC231F93F938__INCLUDED_)
