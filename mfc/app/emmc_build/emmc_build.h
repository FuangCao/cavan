// emmc_build.h : main header file for the EMMC_BUILD application
//

#if !defined(AFX_EMMC_BUILD_H__F8118846_D7E6_44BA_90CF_A4218F9BA970__INCLUDED_)
#define AFX_EMMC_BUILD_H__F8118846_D7E6_44BA_90CF_A4218F9BA970__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEmmc_buildApp:
// See emmc_build.cpp for the implementation of this class
//

class CEmmc_buildApp : public CWinApp
{
public:
	CEmmc_buildApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmmc_buildApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEmmc_buildApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMMC_BUILD_H__F8118846_D7E6_44BA_90CF_A4218F9BA970__INCLUDED_)
