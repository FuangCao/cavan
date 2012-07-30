// emmc_burner.h : main header file for the EMMC_BURNER application
//

#if !defined(AFX_EMMC_BURNER_H__AE36F29E_769B_435F_8052_4D9F4825A844__INCLUDED_)
#define AFX_EMMC_BURNER_H__AE36F29E_769B_435F_8052_4D9F4825A844__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEmmc_burnerApp:
// See emmc_burner.cpp for the implementation of this class
//

class CEmmc_burnerApp : public CWinApp
{
public:
	CEmmc_burnerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmmc_burnerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEmmc_burnerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMMC_BURNER_H__AE36F29E_769B_435F_8052_4D9F4825A844__INCLUDED_)
