// swan_upgradeDlg.h : header file
//

#if !defined(AFX_SWAN_UPGRADEDLG_H__4B8D567C_FFBD_459A_8F21_5205BDF7E6F5__INCLUDED_)
#define AFX_SWAN_UPGRADEDLG_H__4B8D567C_FFBD_459A_8F21_5205BDF7E6F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "edit_imageDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeDlg dialog

class CSwan_upgradeDlg : public CDialog
{
// Construction
public:
	CSwan_upgradeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSwan_upgradeDlg)
	enum { IDD = IDD_SWAN_UPGRADE_DIALOG };
	CProgressCtrl	m_progress_build;
	CListCtrl	m_list_images;
	CString	m_edit_header;
	CString	m_static_status;
	CString	m_edit_version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwan_upgradeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSwan_upgradeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonHeader();
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonDelAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetListText(int iIndex, CSwanImage *pCurrentImage, const CString &rImagePath);
	void LoadImages(CString rPath);
	int ListAddItem(CSwanImage *pImage, const CString &rImagePath);
	int WriteToPackage(CFile &filePkg, LPCTSTR lpszFileName);
	int WriteToPackage(CFile &filePkg, CFile &fileSrc);
	int WriteToPackage(CFile &filePkg, const void *lpBuff, UINT nCount);
	int WriteToPackage(CFile &filePkg, CSwanImage *pImage, LPCTSTR lpszImagePath);
	int WriteHeader(CFile &filePkg, CFileInfo &fileInfo, LPCTSTR lpszHeaderPath);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWAN_UPGRADEDLG_H__4B8D567C_FFBD_459A_8F21_5205BDF7E6F5__INCLUDED_)
