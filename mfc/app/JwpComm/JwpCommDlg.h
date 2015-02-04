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

#include "JwpMcu.h"
#include "JwpCommDesc.h"

typedef enum
{
	JWP_COMM_MSG_BT_STATE_CHANGED = WM_USER + 100,
	JWP_COMM_MSG_UPDATE_DATA,
} jwp_comm_message_t;

/////////////////////////////////////////////////////////////////////////////
// CJwpCommDlg dialog

class CJwpCommDlg : public CDialog, JwpMcu
{
// Construction
public:
	CJwpCommDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CJwpCommDlg)
	enum { IDD = IDD_JWPCOMM_DIALOG };
	CButton	m_ButtonDirectedAdvert;
	CButton	m_ButtonBtState;
	CButton	m_ButtonBtRmPair;
	CButton	m_ButtonBtIdle;
	CEdit	m_EditComCtrl;
	CButton	m_ButtonSendData;
	CButton	m_ButtonSendCommand;
	CButton	m_ButtonDisconnect;
	CButton	m_ButtonConnect;
	CButton	m_ButtonBtDisconnect;
	CButton	m_ButtunBtAdvert;
	JwpCommDesc m_Comm;
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
	afx_msg void OnButtonBtAdvert();
	afx_msg void OnButtonBtDisconnect();
	afx_msg void OnButtonBtIdle();
	afx_msg LRESULT OnBtStateChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonBtRmPair();
	afx_msg void OnButtonClear();
	afx_msg void OnButtonBtState();
	afx_msg void OnButtonDirectedAdvert();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateUiState(void);

protected:
	virtual int HwWrite(const void *buff, jwp_size_t size)
	{
		if (m_Comm.GetPortOpen())
		{
			return m_Comm.HwWrite(buff, size);
		}

		return size;
	}

	virtual void OnDataReceived(const void *buff, jwp_size_t size);
	virtual void OnLogReceived(jwp_device_t device, const char *log, jwp_size_t size);
	virtual void OnCsrStateChanged(const struct jwp_csr_event_state *event);

public:
	void WriteRxData(void)
	{
		m_Comm.WriteRxData();
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JWPCOMMDLG_H__D98B75A4_43FC_486E_B469_8B69FBDBDAD5__INCLUDED_)
