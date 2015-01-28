// JwpUdpDlg.h : header file
//

#if !defined(AFX_JWPUDPDLG_H__A1C21D89_AF3D_4E77_805F_340BC3D96836__INCLUDED_)
#define AFX_JWPUDPDLG_H__A1C21D89_AF3D_4E77_805F_340BC3D96836__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JwpUdpDesc.h"

/////////////////////////////////////////////////////////////////////////////
// CJwpUdpDlg dialog

class CJwpUdpDlg : public CDialog, JwpUdpDesc
{
// Construction
public:
	CJwpUdpDlg(CWnd* pParent = NULL);	// standard constructor	

// Dialog Data
	//{{AFX_DATA(CJwpUdpDlg)
	enum { IDD = IDD_JWPUDP_DIALOG };
	CIPAddressCtrl	m_IpAddressCtrl;
	CString	m_LogValue;
	UINT	m_PortValue;
	CString	m_DataValue;
	CString	m_StateValue;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJwpUdpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CJwpUdpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonSendCommand();
	afx_msg void OnButtonSendData();
	afx_msg void OnButtonClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnSendComplete(void);
	virtual void OnDataReceived(const void *buff, jwp_size_t size);
	virtual void OnCommandReceived(const void *command, jwp_size_t size);
	virtual void OnPackageReceived(const struct jwp_header *hdr);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_JWPUDPDLG_H__A1C21D89_AF3D_4E77_805F_340BC3D96836__INCLUDED_)
