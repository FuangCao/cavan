// JwpComm.h : main header file for the JWPCOMM application
//

#if !defined(AFX_JWPCOMM_H__720513EB_C32A_4661_93DE_5DD8CBAD4C06__INCLUDED_)
#define AFX_JWPCOMM_H__720513EB_C32A_4661_93DE_5DD8CBAD4C06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJwpCommApp:
// See JwpComm.cpp for the implementation of this class
//

class CJwpCommApp : public CWinApp
{
public:
	CJwpCommApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJwpCommApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJwpCommApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JWPCOMM_H__720513EB_C32A_4661_93DE_5DD8CBAD4C06__INCLUDED_)
