// emmc_burnerDlg.h : header file
//

#if !defined(AFX_EMMC_BURNERDLG_H__E6DB9CBC_BA83_430A_90DA_B1F57650DBA7__INCLUDED_)
#define AFX_EMMC_BURNERDLG_H__E6DB9CBC_BA83_430A_90DA_B1F57650DBA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEmmc_burnerDlg dialog

class CEmmc_burnerDlg : public CDialog
{
// Construction
public:
	CEmmc_burnerDlg(CWnd* pParent = NULL);	// standard constructor

	int BurnImage(void);
	void UpdateImageStatus(void);
	int CompareImage(void);

	void ShowMessage(LPCTSTR lpszTitle, UINT nType, LPCTSTR lpzsFormat, va_list ap);
	void ErrorMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...);
	void WarningMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...);
	void RightMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...);
	void SetDlgItemFormatText(int nID, const char *fmt, ...);

// Dialog Data
	//{{AFX_DATA(CEmmc_burnerDlg)
	enum { IDD = IDD_EMMC_BURNER_DIALOG };
	CButton	m_button_compare;
	CEdit	m_edit_file_ctrl;
	CButton	m_button_program;
	CButton	m_button_file;
	CButton	m_button_sel_all;
	CButton	m_button_sel_invert;
	CButton	m_button_reflesh;
	CButton	m_button_desel;
	CCheckListBox	m_listbox_devices;
	CProgressCtrl	m_progress_burn;
	CString	m_edit_file;
	CString	m_static_status;
	CString	m_static_crc32;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmmc_burnerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEmmc_burnerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnButtonFile();
	afx_msg void OnButtonReflesh();
	afx_msg void OnChangeEditFile();
	afx_msg void OnButtonSelAll();
	afx_msg void OnButtonSelInvert();
	afx_msg void OnButtonDesel();
	afx_msg void OnButtonCompare();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD dwImageCRC32;
	void UpdateDevices(void);

	BOOL OpenPhysicalDrive(int nIndex, CFile &fileDevice, UINT nOpenFlags);
	int OpenPhysicalDrives(CFile fileDevices[], int &nCount, UINT nOpenFlags);
	void CloseFiles(CFile files[], int nCount);

	DWORD CRC32(CHAR buff[], DWORD dwCRC, UINT nCount);
	int FileCRC32(CFile &file, DWORD &dwCRC, UINT nLenght);
	int FileCRC32(LPCTSTR lpszFileName, DWORD &dwCRC, UINT nLenght);
	int WriteImage(void);
	int CheckDevices(void);

	int GetDeviceCapability(int iDevIndex, CString &strSize);
	int GetDeviceCapability(int iDevIndex, double &dbSize);
	void SizeToText(ULONGLONG ullSize, CString &strSize);
	void SizeToText(double dbSize, CString &strSize);

	void DisableAllElement(void);
	void EnableAllElement(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMMC_BURNERDLG_H__E6DB9CBC_BA83_430A_90DA_B1F57650DBA7__INCLUDED_)
