// EavooStatisticHelper.cpp : implementation file
//

#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooShortMessage.h"
#include "EavooStatisticHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ================================================================================
// CMonthSell methods
// ================================================================================

CEavooSellMonthNode::CEavooSellMonthNode(int year, int month, int count)
{
	mYear = year;
	mMonth = month;
	mSellCount = count;
}

int CEavooSellMonthNode::ToXmlFileLine(char *buff, const char *prefix, const char *sufix)
{
	return sprintf(buff, "%s<sale date=\"%04d-%02d\" count=\"%d\" />%s", prefix, mYear, mMonth, mSellCount, sufix);
}

int CEavooSellMonthNode::ToTxtFileLine(char *buff, const char *prefix, const char *sufix)
{
	return sprintf(buff, "%sDate: %04d-%02d, Count: %d%s", prefix, mYear, mMonth, mSellCount, sufix);
}

// ================================================================================
// CEavooSellProjectNode methods
// ================================================================================

CEavooSellProjectNode::CEavooSellProjectNode(const char *projectname)
{
	strcpy(mProjectName, projectname);
	mHead = NULL;
}

CEavooSellProjectNode::~CEavooSellProjectNode(void)
{
	FreeLink();
}

void CEavooSellProjectNode::FreeLink(void)
{
	for (CEavooSellMonthNode *q, *p = mHead; p; p = q)
	{
		q = p->next;
		delete p;
	}

	mHead = NULL;
}

CEavooSellMonthNode *CEavooSellProjectNode::FindMonth(int year, int month)
{
	for (CEavooSellMonthNode *p = mHead; p; p = p->next)
	{
		if (p->mYear == year && p->mMonth == month)
		{
			return p;
		}
	}

	return NULL;
}

bool CEavooSellProjectNode::AddMonthSellCount(int year, int month, int count)
{
	CEavooSellMonthNode *p = FindMonth(year, month);
	if (p == NULL)
	{
		p = new CEavooSellMonthNode(year, month, count);
		if (p == NULL)
		{
			return false;
		}

		p->next = mHead;
		mHead = p;
	}
	else
	{
		p->mSellCount += count;
	}

	return true;
}

bool CEavooSellProjectNode::WriteToXmlFile(CFile &file, const char *prefix, const char *sufix)
{
	char buff[1024];
	int length;

	length = sprintf(buff, "%s<project name=\"%s\">%s", prefix, mProjectName, sufix);
	if (CEavooShortMessageHelper::WriteTextToFile(file, buff, length) < 0)
	{
		return false;
	}

	for (CEavooSellMonthNode *p = mHead; p; p = p->next)
	{
		length = p->ToXmlFileLine(buff, "\t\t", "\r\n");
		if (CEavooShortMessageHelper::WriteTextToFile(file, buff, length) < 0)
		{
			return false;
		}
	}

	if (CEavooShortMessageHelper::WriteTextToFile(file, "\t</project>\r\n", -1) < 0)
	{
		return false;
	}

	return true;
}

bool CEavooSellProjectNode::WriteToTxtFile(CFile &file, const char *prefix, const char *sufix)
{
	char buff[1024];
	int length;

	length = sprintf(buff, "%sProject: %s%s", prefix, mProjectName, sufix);
	if (CEavooShortMessageHelper::WriteTextToFile(file, buff, length) < 0)
	{
		return false;
	}

	for (CEavooSellMonthNode *p = mHead; p; p = p->next)
	{
		length = p->ToTxtFileLine(buff, "", "\r\n");
		if (CEavooShortMessageHelper::WriteTextToFile(file, buff, length) < 0)
		{
			return false;
		}
	}

	if (CEavooShortMessageHelper::WriteTextToFile(file, "============================================================\r\n", -1) < 0)
	{
		return false;
	}

	return true;
}

void CEavooSellProjectNode::AppendMonthNode(CEavooSellMonthNode *node)
{
	node->next = NULL;

	if (mHead == NULL)
	{
		mHead = node;
	}
	else
	{
		for (CEavooSellMonthNode *p = mHead; p->next; p = p->next);
		p->next = node;
	}
}

DWORD CEavooSellProjectNode::SaveToFile(CFile &file)
{
	DWORD wrCount = 0, wrLength;

	for (CEavooSellMonthNode *p = mHead; p; wrCount++, p = p->next)
	{
		if (::WriteFile((HANDLE)file.m_hFile, mProjectName, sizeof(mProjectName), &wrLength, NULL) == FALSE || wrLength != sizeof(mProjectName))
		{
			return 0;
		}

		if (::WriteFile((HANDLE)file.m_hFile, (const char *)p, sizeof(CEavooSellMonthNode), &wrLength, NULL) == FALSE || wrLength != sizeof(CEavooSellMonthNode))
		{
			return 0;
		}
	}

	return wrCount;
}

// ================================================================================
// CEavooStatisticHelper methods
// ================================================================================

CEavooStatisticHelper::CEavooStatisticHelper(CProgressCtrl &progress, CStatic &state) : mProgress(progress), mState(state)
{
	mHead = NULL;
}

void CEavooStatisticHelper::FreeLink(void)
{
	CEavooSellProjectNode *head_next;

	while (mHead)
	{
		head_next = mHead->next;
		mHead->FreeLink();
		delete mHead;
		mHead = head_next;
	}
}

CEavooSellProjectNode *CEavooStatisticHelper::FindProject(const char *projectname)
{
	for (CEavooSellProjectNode *p = mHead; p; p = p->next)
	{
		if (strcmp(p->mProjectName, projectname) == 0)
		{
			return p;
		}
	}

	return NULL;
}

bool CEavooStatisticHelper::EavooSellStatisticBase(const char *pathname)
{
	FreeLink();

	CEavooShortMessageHelper helper(mProgress, mState);
	if (helper.Initialize(CFile::modeRead | CFile::shareDenyNone) == false)
	{
		return false;
	}

	DWORD dwLoadCount;
	struct tm *time;
	char projectname[16];
	CEavooShortMessageBody body;
	CEavooSellProjectNode *project;
	DWORD dwTotalLength = helper.GetFileLength();
	double dbReadLength, percent;
	unsigned char count;
	char buff[8];

	dwLoadCount = LoadFromFile();
	if (dwLoadCount)
	{
		DWORD offset = dwLoadCount * sizeof(CEavooShortMessage);
		if (offset == dwTotalLength)
		{
			return true;
		}
		else if (offset > dwTotalLength)
		{
			FreeLink();
		}
		else
		{
			offset = helper.Seek(offset);
			if (offset == (DWORD)-1)
			{
				FreeLink();
			}
			else
			{
				dwTotalLength -= offset;
			}
		}
	}

	if (dwTotalLength == 0)
	{
		return true;
	}

	mShouldStop = false;
	dbReadLength = 0;

	for (count = 0; ; count++)
	{
		if (mShouldStop)
		{
			AfxMessageBox("取消统计");
			mState.SetWindowText("取消统计");
			return false;
		}

		DWORD length = helper.ReadFromFile();
		if ((count & PROGRESS_MIN_COUNT) == 0 || length == 0)
		{
			percent = dbReadLength * 100 / dwTotalLength;
			mProgress.SetPos((int)percent);
			sprintf(buff, "%0.2lf%%", percent);
			mState.SetWindowText(buff);

			if (length == 0)
			{
				break;
			}
		}

		dbReadLength += length;

		time = gmtime(helper.GetDate());
		if (time == NULL)
		{
			AfxMessageBox("日期无效");
			mState.SetWindowText("日期无效");
			return false;
		}

		if (helper.ParseBody(body) == false)
		{
			AfxMessageBox("短信的内容非法");
			mState.SetWindowText("短信的内容非法");
			return false;
		}

		const char *p_name = body.GetProjectName(projectname);
		if (p_name == NULL)
		{
			p_name = "OTHER";
		}

		project = FindProject(p_name);
		if (project == NULL)
		{
			project = new CEavooSellProjectNode(p_name);
			if (project == NULL)
			{
				AfxMessageBox("无法分配存储空间");
				mState.SetWindowText("无法分配存储空间");
				return false;
			}

			project->next = mHead;
			mHead = project;
		}

		if (project->AddMonthSellCount(time->tm_year + 1900, time->tm_mon + 1, 1) == false)
		{
			return false;
		}
	}

	return dbReadLength == dwTotalLength;
}

bool CEavooStatisticHelper::EavooSellStatistic(const char *pathname, CTabCtrl &tab)
{
	if (EavooSellStatisticBase(pathname))
	{
		tab.DeleteAllItems();

		for (CEavooSellProjectNode *p = mHead; p; p = p->next)
		{
			tab.InsertItem(0, p->mProjectName);
		}

		SaveToFile();
		return true;
	}

	FreeLink();

	return false;
}

void CEavooStatisticHelper::ShouldStop(void)
{
	mShouldStop = true;
}

bool CEavooStatisticHelper::ShowProject(CEavooSellProjectNode *head, CListCtrl &list)
{
	int total = 0;
	char buff[32];

	list.DeleteAllItems();

	for (CEavooSellMonthNode *p = head->mHead; p; p = p->next)
	{
		sprintf(buff, "%04d年%02d月", p->mYear, p->mMonth);
		int index = list.InsertItem(list.GetItemCount(), buff);
		sprintf(buff, "%d 台", p->mSellCount);
		list.SetItemText(index, 1, buff);
		total += p->mSellCount;
	}

	sprintf(buff, "总销量：%d 台", total);
	mState.SetWindowText(buff);

	return true;
}

bool CEavooStatisticHelper::ShowProject(const char *projectname, CListCtrl &list)
{
	if (projectname == NULL)
	{
		return false;
	}

	return ShowProject(FindProject(projectname), list);
}

bool CEavooStatisticHelper::ExportXmlFile(CFile &file)
{
	if (CEavooShortMessageHelper::WriteTextToFile(file, "<?xml version=\"1.0\" encoding=\"ascii\" ?>\r\n", -1) < 0)
	{
		return false;
	}

	if (CEavooShortMessageHelper::WriteTextToFile(file, "<projects>\r\n", -1) < 0)
	{
		return false;
	}

	for (CEavooSellProjectNode *p = mHead; p; p = p->next)
	{
		if (p->WriteToXmlFile(file, "\t", "\r\n") == false)
		{
			return false;
		}
	}

	if (CEavooShortMessageHelper::WriteTextToFile(file, "</projects>", -1) < 0)
	{
		return false;
	}

	return true;
}

bool CEavooStatisticHelper::ExportTxtFile(CFile &file)
{
	if (CEavooShortMessageHelper::WriteTextToFile(file, "============================================================\r\n", -1) < 0)
	{
		return false;
	}

	for (CEavooSellProjectNode *p = mHead; p; p = p->next)
	{
		if (p->WriteToTxtFile(file, "", "\r\n") == false)
		{
			return false;
		}
	}

	return true;
}

void CEavooStatisticHelper::AppendProjectNode(CEavooSellProjectNode *node)
{
	node->next = NULL;

	if (mHead == NULL)
	{
		mHead = node;
	}
	else
	{
		for (CEavooSellProjectNode *p = mHead; p->next; p = p->next);
		p->next = node;
	}
}

DWORD CEavooStatisticHelper::LoadFromFile(const char *pathname)
{
	CFile file;
	if (file.Open(pathname, CFile::modeRead, NULL) == false)
	{
		return 0;
	}

	DWORD result = LoadFromFile(file);
	file.Close();

	return result;
}

DWORD CEavooStatisticHelper::LoadFromFile(CFile &file)
{
	CEavooSellProjectNode *nodeProject;
	char projectName[MAX_PROJECT_NAME_LENGTH];
	DWORD rdLength, ldCount;

	FreeLink();
	nodeProject = NULL;
	ldCount = 0;

	while (1)
	{
		if (::ReadFile((HANDLE)file.m_hFile, projectName, MAX_PROJECT_NAME_LENGTH, &rdLength, NULL) == FALSE || rdLength != MAX_PROJECT_NAME_LENGTH)
		{
			break;
		}

		CEavooSellMonthNode *nodeMonth = new CEavooSellMonthNode();
		if (projectName[0] == 0 || nodeMonth == NULL)
		{
			FreeLink();
			return false;
		}

		if (::ReadFile((HANDLE)file.m_hFile, (char *)nodeMonth, sizeof(CEavooSellMonthNode), &rdLength, NULL) == FALSE || rdLength != sizeof(CEavooSellMonthNode))
		{
			FreeLink();
			return 0;
		}

		if (nodeProject == NULL || strcmp(nodeProject->mProjectName, projectName))
		{
			nodeProject = new CEavooSellProjectNode(projectName);
			if (nodeProject == NULL)
			{
				delete nodeMonth;
				FreeLink();
				return 0;
			}

			AppendProjectNode(nodeProject);
		}

		nodeProject->AppendMonthNode(nodeMonth);
		ldCount += nodeMonth->mSellCount;
	}

	return ldCount;
}

DWORD CEavooStatisticHelper::SaveToFile(const char *pathname)
{
	CFile file;
	if (file.Open(pathname, CFile::modeWrite | CFile::modeCreate, NULL) == false)
	{
		return 0;
	}

	DWORD result = SaveToFile(file);
	file.Close();

	return result;
}

DWORD CEavooStatisticHelper::SaveToFile(CFile &file)
{
	DWORD wrCount, wrTotal = 0;

	for (CEavooSellProjectNode *p = mHead; p; p = p->next)
	{
		wrCount = p->SaveToFile(file);
		if (wrCount == 0)
		{
			break;
		}

		wrTotal += wrCount;
	}

	return wrTotal;
}
