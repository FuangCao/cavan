#if !defined(AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
#define AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMonthSellNode
{
public:
	int mYear, mMonth;
	ULONG mSellCount;
	CMonthSellNode *next;

public:
	CMonthSellNode(int year, int month, int count = 0);
};

class CMonthSellLink
{
public:
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

class CEavooStatisticHelper
{
private:
	CMonthSellLink *mHead;
	bool mShouldStop;
	CProgressCtrl &mProgress;
	CStatic &mState;

public:
	CEavooStatisticHelper();
	void ShouldStop(void);
	void FreeLink(void);
	CMonthSellLink *FindProject(const char *projectname);
	bool EavooSellStatisticBase(const char *pathname);
	bool EavooSellStatistic(const char *pathname);

	bool ShowProject(const char *projectname);
	bool ShowProject(CMonthSellLink *head);
	bool ShowProject(void);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
