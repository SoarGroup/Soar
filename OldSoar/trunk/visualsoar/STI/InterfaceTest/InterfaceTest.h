// InterfaceTest.h : main header file for the INTERFACETEST application
//

#if !defined(AFX_INTERFACETEST_H__9E9196E2_0646_47CE_8AC5_012552399A4D__INCLUDED_)
#define AFX_INTERFACETEST_H__9E9196E2_0646_47CE_8AC5_012552399A4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTInterfaceTestApp:
// See InterfaceTest.cpp for the implementation of this class
//

class CTInterfaceTestApp : public CWinApp
{
public:
	CTInterfaceTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTInterfaceTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTInterfaceTestApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERFACETEST_H__9E9196E2_0646_47CE_8AC5_012552399A4D__INCLUDED_)
