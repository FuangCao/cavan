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
	char mMonth[16];
	int mSellCount;
	CMonthSellNode *next;

public:
	CMonthSellNode(const char *month);
};

class CMonthSellLink
{
	friend CEavooSellStatisticDlg2;

private:
	CMonthSellNode *mHead;

public:
	CMonthSellLink(void) : mHead(NULL) {}
	~CMonthSellLink(void);
	void InitLinkHead(void);
	CMonthSellNode *FindMonth(const char *month);
	bool AddMonthSellCount(const char *month, int count);
};

class CEavooSellStatisticDlg2 : public CDialog
{
// Construction
public:
	CEavooSellStatisticDlg2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEavooSellStatisticDlg2)
	enum { IDD = IDD_DIALOG_statistic };
	CListCtrl	m_list_sell;
	CStatic	m_static_total_sell;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CMonthSellLink mLink;

private:
	char *AdbParseDataMulti(const char *buff, const char *end);
	char *AdbParseDataMain(char *buff, char *end);
	bool EavooSellParseFile(const char *filename);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSELLSTATISTICDLG2_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
