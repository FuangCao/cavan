// ProxyServerDlg.h : header file
//

#if !defined(AFX_PROXYSERVERDLG_H__11AE879C_5A3C_45CE_8E4E_AE9312B65D21__INCLUDED_)
#define AFX_PROXYSERVERDLG_H__11AE879C_5A3C_45CE_8E4E_AE9312B65D21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CProxyServerDlg dialog

#include "ProxyService.h"

class CProxyServerDlg : public CDialog
{
private:
	CProxyService mProxyService;
// Construction
public:
	CProxyServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CProxyServerDlg)
	enum { IDD = IDD_PROXYSERVER_DIALOG };
	CButton	m_ctrlButtonStop;
	CButton	m_ctrlButtonStart;
	CIPAddressCtrl	m_ctrlProxyIP;
	int		m_nLocalProtocol;
	int		m_nProxyProtocol;
	short	m_dwLocalPort;
	short	m_dwProxyPort;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProxyServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProxyServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CProxyProcotolType ValueToProtocolType(int value);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROXYSERVERDLG_H__11AE879C_5A3C_45CE_8E4E_AE9312B65D21__INCLUDED_)
