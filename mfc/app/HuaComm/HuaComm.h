// HuaComm.h : main header file for the HUACOMM application
//

#if !defined(AFX_HUACOMM_H__13F1511A_679D_4BEB_8BEB_EED1F5D00906__INCLUDED_)
#define AFX_HUACOMM_H__13F1511A_679D_4BEB_8BEB_EED1F5D00906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHuaCommApp:
// See HuaComm.cpp for the implementation of this class
//

class CHuaCommApp : public CWinApp
{
public:
	CHuaCommApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHuaCommApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHuaCommApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class HuaComm
{
private:
	handle_t m_hComm;
	handle_t m_hRdThread;
	handle_t m_hMutex;
	handle_t m_hRdEvent;
	handle_t m_hWrEvent;
	int m_nCommPort;
	char *head, *tail, *last;
	char m_RdDataPool[4096];
	
	static DWORD WINAPI PollThread(LPVOID pvarg);

public:
	HuaComm(int port);
	bool IsOpen(void);
	bool Open(void);
	void Close(void);
	void Poll(void);
	int Read(void *buff, size_t size, int timeout);
	int Write(const void *buff, size_t size);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUACOMM_H__13F1511A_679D_4BEB_8BEB_EED1F5D00906__INCLUDED_)
