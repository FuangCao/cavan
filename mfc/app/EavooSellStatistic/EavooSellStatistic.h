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
#include "afxmt.h"
#include "EavooShortmessage.h"

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticApp:
// See EavooSellStatistic.cpp for the implementation of this class
//

class CEavooSellStatisticApp : public CWinApp
{
public:
	char mDatabasePath[1024];
	CMutex mDatabaseMutex;
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

public:
	bool OpenDatabase(CFile &file, const char *pathname, UINT nOpenFlags);
	DWORD WriteDatabase(CFile &file, const char *buff, DWORD length);
	DWORD ReadDatabase(CFile &file, char *buff, DWORD length);

	DWORD ReadDatabaseNolock(CFile &file, char *buff, DWORD length);
	DWORD ReadDatabaseText(CFile &file, char *buff, int size);
	DWORD ReadDatabaseOld(CFile &file, CEavooShortMessage &message);
};

extern CEavooSellStatisticApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSELLSTATISTIC_H__B32B58BD_E3F8_49B3_931D_45130E4377FD__INCLUDED_)
