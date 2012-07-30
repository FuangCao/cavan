#if !defined(AFX_EDIT_IMAGEDLG_H__1E93247A_A75D_4585_9E77_0B94A64B5AE5__INCLUDED_)
#define AFX_EDIT_IMAGEDLG_H__1E93247A_A75D_4585_9E77_0B94A64B5AE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Edit_imageDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CEdit_imageDlg dialog

class CEdit_imageDlg : public CDialog
{
// Construction
public:
	CEdit_imageDlg(struct CSwanImage *pCurrentImage = NULL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEdit_imageDlg)
	enum { IDD = IDD_DIALOG_EDIT_IMAGE };
	CComboBox	m_combo_type;
	CComboBox	m_combo_offset;
	CComboBox	m_combo_minor;
	CComboBox	m_combo_major;
	CComboBox	m_combo_device;
	CString	m_edit_path;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEdit_imageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEdit_imageDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboType();
	afx_msg void OnButtonPath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SelectImage(struct CSwanImage *pCurrentImage);
public:
	struct CSwanImage *pCurrentImage;
	UINT nOldLength;
	static CString &SizeToText(UINT nSize);
	static UINT TextToSize(const char cpText[]);
	static UINT UnitsToSize(char cUnit);
	static UINT TextToValue(const char cpText[]);
	static CString &ValueToText(UINT nValue);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_IMAGEDLG_H__1E93247A_A75D_4585_9E77_0B94A64B5AE5__INCLUDED_)
