// EavooSellStatistic.h : main header file for the EAVOOSELLSTATISTIC application
//

#if !defined(AFX_EAVOOSELLSTATISTIC_H__B32B58BD_E3F8_49B3_931D_45130E4377FD__INCLUDED_)
#define AFX_EAVOOSELLSTATISTIC_H__B32B58BD_E3F8_49B3_931D_45130E4377FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticApp:
// See EavooSellStatistic.cpp for the implementation of this class
//

class CEavooSellStatisticApp : public CWinApp
{
public:
	CEavooSellStatisticApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEavooSellStatisticApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEavooSellStatisticApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSELLSTATISTIC_H__B32B58BD_E3F8_49B3_931D_45130E4377FD__INCLUDED_)
