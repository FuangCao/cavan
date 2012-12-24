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

class CProxyThread;
class CProxyService;

class CProxyServerDlg : public CDialog
{
private:
	CProxyService *mProxyService;
	bool mServiceRunning;
	NOTIFYICONDATA mNotifyIconData;
	CMenu mProxyMenu;
// Construction
public:
	CProxyServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CProxyServerDlg)
	enum { IDD = IDD_PROXYSERVER_DIALOG };
	CButton	m_ctrlRadioProxyProtocol;
	CStatic	m_ctrlStatus;
	CProgressCtrl	m_ctrlServiceProgress;
	CListCtrl	m_ctrlListService;
	CIPAddressCtrl	m_ctrlProxyIP;
	int		m_nLocalProtocol;
	int		m_nProxyProtocol;
	short	m_dwLocalPort;
	short	m_dwProxyPort;
	DWORD	m_dwDaemonCount;
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
	afx_msg void OnRadioProxyProtocol();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNotifyIconProxy(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMenuitemExit();
	afx_msg void OnMenuitemVisible();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CProxyProcotolType ValueToProtocolType(int value);
	void ShowStatus(const char *strFormat, ...);
	void EnableAllWindow(bool enable);
	bool StartAdbServer(void);
	bool EnableService(bool enable);

	static void StartThreadHandler(void *data);
	static void StopThreadHandler(void *data);

	friend CProxyThread;
	friend CProxyService;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROXYSERVERDLG_H__11AE879C_5A3C_45CE_8E4E_AE9312B65D21__INCLUDED_)
