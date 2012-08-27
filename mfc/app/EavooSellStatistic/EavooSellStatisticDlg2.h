#if !defined(AFX_EAVOOSELLSTATISTICDLG2_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
#define AFX_EAVOOSELLSTATISTICDLG2_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EavooSellStatisticDlg2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg2 dialog
class CMonthSellLink;
class CEavooSellStatisticDlg2;

class CMonthSellNode
{
	friend CMonthSellLink;
	friend CEavooSellStatisticDlg2;

private:
	int mYear, mMonth;
	ULONG mSellCount;
	CMonthSellNode *next;

public:
	CMonthSellNode(int year, int month, int count = 0);
};

class CMonthSellLink
{
	friend CEavooSellStatisticDlg2;

private:
	char mProjectName[16];
	CMonthSellNode *mHead;
	CMonthSellLink *next;

public:
	CMonthSellLink(const char *projectname);
	~CMonthSellLink(void);
	void FreeLink(void);
	CMonthSellNode *FindMonth(int year, int month);
	bool AddMonthSellCount(int year, int month, int count);
};

class CEavooSellStatisticDlg2 : public CDialog
{
// Construction
public:
	CEavooSellStatisticDlg2(CWnd* pParent = NULL);   // standard constructor
	~CEavooSellStatisticDlg2();

// Dialog Data
	//{{AFX_DATA(CEavooSellStatisticDlg2)
	enum { IDD = IDD_DIALOG_statistic };
	CButton	m_button_stop;
	CButton	m_button_ok;
	CStatic	m_static_status;
	CProgressCtrl	m_progress_statistic;
	CTabCtrl	m_tab_sell;
	CListCtrl	m_list_sell;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEavooSellStatisticDlg2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEavooSellStatisticDlg2)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTABsell(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBUTTONstop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CWinThread *mThread;
	CMonthSellLink *mHead;
	bool mShouldStop;

public:
	CMonthSellLink *FindProject(const char *projectname);
	bool EavooSellStatisticBase(const char *pathname);
	bool EavooSellStatistic(const char *pathname);
	bool ShowProject(const char *projectname);
	bool ShowProject(CMonthSellLink *head);
	bool ShowProject(void);
	void FreeLink(void);

	static int ThreadHandler(void *data);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSELLSTATISTICDLG2_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
