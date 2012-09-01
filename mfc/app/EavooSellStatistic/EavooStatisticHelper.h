#if !defined(AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
#define AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_PROJECT_NAME_LENGTH	16

class CEavooSellMonthNode
{
public:
	int mYear, mMonth;
	ULONG mSellCount;
	CEavooSellMonthNode *next;

public:
	CEavooSellMonthNode(void) {}
	CEavooSellMonthNode(int year, int month, int count = 0);
	int ToXmlFileLine(char *buff, const char *prefix, const char *sufix);
	int ToTxtFileLine(char *buff, const char *prefix, const char *sufix);
};

class CEavooSellProjectNode
{
public:
	char mProjectName[MAX_PROJECT_NAME_LENGTH];
	CEavooSellMonthNode *mHead;
	CEavooSellProjectNode *next;

public:
	CEavooSellProjectNode(void) {};
	CEavooSellProjectNode(const char *projectname);
	~CEavooSellProjectNode(void);
	void FreeLink(void);
	CEavooSellMonthNode *FindMonth(int year, int month);
	bool AddMonthSellCount(int year, int month, int count);
	void AppendMonthNode(CEavooSellMonthNode *node);
	bool WriteToXmlFile(CFile &file, const char *prefix, const char *sufix);
	bool WriteToTxtFile(CFile &file, const char *prefix, const char *sufix);

	DWORD SaveToFile(CFile &file);
};

class CEavooStatisticHelper
{
private:
	CEavooSellProjectNode *mHead;
	bool mShouldStop;
	CProgressCtrl &mProgress;
	CStatic &mState;

public:
	CEavooStatisticHelper(CProgressCtrl &progress, CStatic &state);
	void ShouldStop(void);
	void FreeLink(void);
	CEavooSellProjectNode *FindProject(const char *projectname);
	void AppendProjectNode(CEavooSellProjectNode *node);
	bool EavooSellStatisticBase(const char *pathname);
	bool EavooSellStatistic(const char *pathname, CTabCtrl &table);

	bool ShowProject(CEavooSellProjectNode *head, CListCtrl &list);
	bool ShowProject(const char *projectname, CListCtrl &list);

	bool ExportXmlFile(CFile &file);
	bool ExportTxtFile(CFile &file);

	DWORD LoadFromFile(const char *pathname = theApp.mStatisticPath);
	DWORD LoadFromFile(CFile &file);
	DWORD SaveToFile(const char *pathname = theApp.mStatisticPath);
	DWORD SaveToFile(CFile &file);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EAVOOSTATISTICHELPER_H__E551305C_8E5B_4E3D_88EA_FF83E8231013__INCLUDED_)
