#if !defined(AFX_RESULTS1_H__6DDFD42E_CB07_4B40_B57C_8173DF8F04A4__INCLUDED_)
#define AFX_RESULTS1_H__6DDFD42E_CB07_4B40_B57C_8173DF8F04A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Results1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTResults1 dialog

class CTResults1 : public CDialog
{
// Construction
public:
	CTResults1(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTResults1)
	enum { IDD = IDD_RESULTS1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTResults1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTResults1)
	afx_msg void OnWindiff1Runtime1();
	afx_msg void OnWindiff1Runtime2();
	virtual BOOL OnInitDialog();
	afx_msg void OnWindiff1Tool1();
	afx_msg void OnWindiff2Runtime1();
	afx_msg void OnWindiff2Runtime2();
	afx_msg void OnWindiff2Tool1();
	afx_msg void OnWindiff3Runtime1();
	afx_msg void OnWindiff3Runtime2();
	afx_msg void OnWindiff3Tool1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESULTS1_H__6DDFD42E_CB07_4B40_B57C_8173DF8F04A4__INCLUDED_)
