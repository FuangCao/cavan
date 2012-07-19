// EavooSellStatisticDlg.h : header file
//

#if !defined(AFX_EAVOOSELLSTATISTICDLG_H__3F41B8B2_168E_4DA3_99E0_94F704ED7CD6__INCLUDED_)
#define AFX_EAVOOSELLSTATISTICDLG_H__3F41B8B2_168E_4DA3_99E0_94F704ED7CD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_SERVER_IP	"127.0.0.1"
#define DEFAULT_SERVER_PORT	8888
#define CACHE_FILENAME		"eavoo_sell.txt"

#define ADB_SERVICE_PORT	5037
#define ADB_STATE_OKAY		"OKAY"
#define ADB_STATE_FAIL		"FAIL"

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg dialog

class CEavooSellStatisticDlg : public CDialog
{
// Construction
public:
	CEavooSellStatisticDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CEavooSellStatisticDlg)
	enum { IDD = IDD_EAVOOSELLSTATISTIC_DIALOG };
	CIPAddressCtrl	m_ipaddress1;
	CListCtrl	m_list_sms;
	UINT	m_edit_port;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEavooSellStatisticDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEavooSellStatisticDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBUTTONconnect();
	afx_msg void OnBUTTONdisconnect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	SOCKET mSocket;
	WSADATA mWsaData;
	CFile mFile;
	char mAdbStatus[1024];

public:
	static int ThreadHandler(void *data);
	int TextCmpLH(const char *left, const char *right);
	int AdbReadStatus(void);
	int AdbSendText(const char *text);
	int AdbSendCommand(const char *command);
	void CloseSocket(void);
	int AdbConnect(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSELLSTATISTICDLG_H__3F41B8B2_168E_4DA3_99E0_94F704ED7CD6__INCLUDED_)
