// emmc_buildDlg.h : header file
//

#if !defined(AFX_EMMC_BUILDDLG_H__954ACD1E_2355_4AAD_BD2C_6A918262DA22__INCLUDED_)
#define AFX_EMMC_BUILDDLG_H__954ACD1E_2355_4AAD_BD2C_6A918262DA22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEmmc_buildDlg dialog

class CEmmc_buildDlg : public CDialog
{
// Construction
public:
	CEmmc_buildDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CEmmc_buildDlg)
	enum { IDD = IDD_EMMC_BUILD_DIALOG };
	CProgressCtrl	m_progress_build;
	CString	m_edit_busybox;
	CString	m_edit_uboot;
	CString	m_edit_uimage;
	CString	m_static_status;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmmc_buildDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEmmc_buildDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnButtonUboot();
	afx_msg void OnButtonUimage();
	afx_msg void OnButtonBusybox();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int BuileFactoryImage(CString &csFileName);
	int AppendFileToImage(LPCTSTR lpszFileName, CFile &fileImage, LONG lOffset);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMMC_BUILDDLG_H__954ACD1E_2355_4AAD_BD2C_6A918262DA22__INCLUDED_)
