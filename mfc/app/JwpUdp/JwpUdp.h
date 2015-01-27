// JwpUdp.h : main header file for the JWPUDP application
//

#if !defined(AFX_JWPUDP_H__9873F876_5292_4907_BA2B_A98641638228__INCLUDED_)
#define AFX_JWPUDP_H__9873F876_5292_4907_BA2B_A98641638228__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJwpUdpApp:
// See JwpUdp.cpp for the implementation of this class
//

class CJwpUdpApp : public CWinApp
{
public:
	CJwpUdpApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJwpUdpApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJwpUdpApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JWPUDP_H__9873F876_5292_4907_BA2B_A98641638228__INCLUDED_)
