// sdcard_burnerDlg.h : header file
//

#if !defined(AFX_SDCARD_BURNERDLG_H__55B49D8F_4C5D_453E_8C8D_0724AD8F9016__INCLUDED_)
#define AFX_SDCARD_BURNERDLG_H__55B49D8F_4C5D_453E_8C8D_0724AD8F9016__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSdcard_burnerDlg dialog

#include <Cavan.h>

class CSdcard_burnerDlg : public CDialog
{
// Construction
public:
	CSdcard_burnerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSdcard_burnerDlg)
	enum { IDD = IDD_SDCARD_BURNER_DIALOG };
	CComboBox	m_combo_skip_ctrl;
	CComboBox	m_combo_seek_ctrl;
	CComboBox	m_combo_count_ctrl;
	CComboBox	m_combo_bs_ctrl;
	CProgressCtrl	m_progress_burn_ctrl;
	CComboBox	m_combo_device_ctrl;
	CComboBox	m_combo_uboot_ctrl;
	CString	m_edit_uboot_val;
	CString	m_edit_uimage_val;
	CString	m_edit_uramdisk_val;
	CString	m_static_status_val;
	CString	m_edit_if_val;
	UINT	m_edit_seek_uint;
	UINT	m_edit_bs_uint;
	UINT	m_edit_count_uint;
	UINT	m_edit_skip_uint;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSdcard_burnerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSdcard_burnerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonUboot();
	afx_msg void OnButtonUimage();
	afx_msg void OnButtonUramdisk();
	virtual void OnOK();
	afx_msg void OnButtonFactory();
	afx_msg void OnButtonRefresh();
	afx_msg void OnButtonIf();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateDevice(void);
	int BurnNormalImage(CCavanDevice &cvnDevice);
	int WritePartitionTable(int iDevIndex, ULONGLONG ullStartByte);
	int WriteImages(int iDevIndex);
	int GetTargetDeviceIndex(void);
	BOOL FormatDevicePartitions(int iDevIndex);
	void PartInfoToString(PPARTITION_INFORMATION pPartInfo, CString &strPartInfo);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SDCARD_BURNERDLG_H__55B49D8F_4C5D_453E_8C8D_0724AD8F9016__INCLUDED_)
