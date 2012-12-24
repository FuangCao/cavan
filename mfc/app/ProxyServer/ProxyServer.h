// ProxyServer.h : main header file for the PROXYSERVER application
//

#if !defined(AFX_PROXYSERVER_H__46CF4C33_4D3A_42B3_9715_E6B0F1BE9608__INCLUDED_)
#define AFX_PROXYSERVER_H__46CF4C33_4D3A_42B3_9715_E6B0F1BE9608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define CavanMessageBoxPos(func) \
	CavanMessageBoxInfo("%s[%d]", func, __LINE__)

/////////////////////////////////////////////////////////////////////////////
// CProxyServerApp:
// See ProxyServer.cpp for the implementation of this class
//

class CProxyServerApp : public CWinApp
{
public:
	CProxyServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProxyServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CProxyServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

int CavanMessageBox(UINT nType, const char *strFormat, va_list ap);
void CavanMessageBoxError(const char *strFormat, ...);
void CavanMessageBoxWarning(const char *strFormat, ...);
void CavanMessageBoxInfo(const char *strFormat, ...);
bool CavanMessageBoxYesNo(const char *strFormat, ...);

int TextLhCmp(const char *left, const char *right);
int CharToValue(char c);
DWORD TextToValue(const char *text, int base);
bool ExecuteCommand(const char *strCommandName, const char *strArgFormat, ...);

#endif // !defined(AFX_PROXYSERVER_H__46CF4C33_4D3A_42B3_9715_E6B0F1BE9608__INCLUDED_)
