// TestMaster.h : main header file for the TESTMASTER application
//

#if !defined(AFX_TESTMASTER_H__DC236150_E4E3_431F_828E_AF4C55C91B95__INCLUDED_)
#define AFX_TESTMASTER_H__DC236150_E4E3_431F_828E_AF4C55C91B95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

void   GetFilePath(TCHAR* pPath, long size, TCHAR const* pFileName) ;
TCHAR* GetLogName(TCHAR const* pLogType, bool bRuntime, long testNumber,
				long logNumber, TCHAR* pOutput) ;

/////////////////////////////////////////////////////////////////////////////
// CTestMasterApp:
// See TestMaster.cpp for the implementation of this class
//

class CTestMasterApp : public CWinApp
{
public:
	CTestMasterApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestMasterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTestMasterApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTMASTER_H__DC236150_E4E3_431F_828E_AF4C55C91B95__INCLUDED_)
