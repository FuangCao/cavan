// sdcard_burner.h : main header file for the SDCARD_BURNER application
//

#if !defined(AFX_SDCARD_BURNER_H__8939EA6E_08DB_407B_BB47_48528E0CFF44__INCLUDED_)
#define AFX_SDCARD_BURNER_H__8939EA6E_08DB_407B_BB47_48528E0CFF44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSdcard_burnerApp:
// See sdcard_burner.cpp for the implementation of this class
//

class CSdcard_burnerApp : public CWinApp
{
public:
	CSdcard_burnerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSdcard_burnerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSdcard_burnerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SDCARD_BURNER_H__8939EA6E_08DB_407B_BB47_48528E0CFF44__INCLUDED_)
