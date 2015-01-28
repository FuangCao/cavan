// JwpCommDlg.h : header file
//
//{{AFX_INCLUDES()
#include "mscomm.h"
//}}AFX_INCLUDES

#if !defined(AFX_JWPCOMMDLG_H__D98B75A4_43FC_486E_B469_8B69FBDBDAD5__INCLUDED_)
#define AFX_JWPCOMMDLG_H__D98B75A4_43FC_486E_B469_8B69FBDBDAD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JwpCore.h"

/////////////////////////////////////////////////////////////////////////////
// CJwpCommDlg dialog

class CJwpCommDlg : public CDialog, JwpCore
{
// Construction
public:
	CJwpCommDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CJwpCommDlg)
	enum { IDD = IDD_JWPCOMM_DIALOG };
	CMSComm	m_Comm;
	UINT	m_ComNum;
	CString	m_StateValue;
	CString	m_EditCommand;
	CString	m_EditLog;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJwpCommDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	virtual int HwWrite(const void *buff, jwp_size_t size);

	// Generated message map functions
	//{{AFX_MSG(CJwpCommDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnect();
	afx_msg void OnButtonDisconnect();
	afx_msg void OnOnCommMscomm();
	afx_msg void OnButtonSendCommand();
	afx_msg void OnButtonSendData();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JWPCOMMDLG_H__D98B75A4_43FC_486E_B469_8B69FBDBDAD5__INCLUDED_)
