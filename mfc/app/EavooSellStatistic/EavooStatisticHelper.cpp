#include "EavooStatisticHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CMonthSell methods

CMonthSellNode::CMonthSellNode(int year, int month, int count)
{
	mYear = year;
	mMonth = month;
	mSellCount = count;
}

CMonthSellLink::CMonthSellLink(const char *projectname)
{
	strcpy(mProjectName, projectname);
	mHead = NULL;
}

CMonthSellLink::~CMonthSellLink(void)
{
	FreeLink();
}

void CMonthSellLink::FreeLink(void)
{
	for (CMonthSellNode *q, *p = mHead; p; p = q)
	{
		q = p->next;
		delete p;
	}

	mHead = NULL;
}

CMonthSellNode *CMonthSellLink::FindMonth(int year, int month)
{
	for (CMonthSellNode *p = mHead; p; p = p->next)
	{
		if (p->mYear == year && p->mMonth == month)
		{
			return p;
		}
	}

	return NULL;
}

bool CMonthSellLink::AddMonthSellCount(int year, int month, int count)
{
	CMonthSellNode *p = FindMonth(year, month);
	if (p == NULL)
	{
		p = new CMonthSellNode(year, month, count);
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

/////////////////////////////////////////////////////////////////////////////
// CEavooStatisticHelper methods

CEavooStatisticHelper::CEavooStatisticHelper()
{
}

void CEavooStatisticHelper::FreeLink(void)
{
	CMonthSellLink *head_next;

	while (mHead)
	{
		head_next = mHead->next;
		mHead->FreeLink();
		delete mHead;
		mHead = head_next;
	}
}

CMonthSellLink *CEavooStatisticHelper::FindProject(const char *projectname)
{
	for (CMonthSellLink *p = mHead; p; p = p->next)
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

	CEavooShortMessageHelper helper;
	if (helper.Initialize(CFile::modeRead | CFile::shareDenyNone) == false)
	{
		return false;
	}

	struct tm *time;
	char projectname[16];
	CEavooShortMessageBody body;
	CMonthSellLink *project;
	DWORD dwTotalLength = helper.GetFileLength();
	double dbReadLength, percent;
	unsigned char count;
	char buff[8];

	m_progress_statistic.SetRange(0, 100);
	mShouldStop = false;
	dbReadLength = 0;

	for (count = 0; ; count++)
	{
		if (mShouldStop)
		{
			AfxMessageBox("取消统计");
			m_static_status.SetWindowText("取消统计");
			return false;
		}

		DWORD length = helper.ReadFromFile();
		if ((count & PROGRESS_MIN_COUNT) == 0 || length == 0)
		{
			percent = dbReadLength * 100 / dwTotalLength;
			m_progress_statistic.SetPos((int)percent);
			sprintf(buff, "%0.2lf%%", percent);
			m_static_status.SetWindowText(buff);

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
			m_static_status.SetWindowText("日期无效");
			return false;
		}

		if (helper.ParseBody(body) == false)
		{
			AfxMessageBox("短信的内容非法");
			m_static_status.SetWindowText("短信的内容非法");
			return false;
		}

		const char *p_name = body.GetProjectName(projectname);
		if (p_name == NULL)
		{
			p_name = "未知";
		}

		project = FindProject(p_name);
		if (project == NULL)
		{
			project = new CMonthSellLink(p_name);
			if (project == NULL)
			{
				AfxMessageBox("无法分配存储空间");
				m_static_status.SetWindowText("无法分配存储空间");
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

bool CEavooStatisticHelper::EavooSellStatistic(const char *pathname)
{
	if (EavooSellStatisticBase(pathname))
	{
		for (CMonthSellLink *p = mHead; p; p = p->next)
		{
			m_tab_sell.InsertItem(0, p->mProjectName);
		}

		if (mHead)
		{
			ShowProject();
		}

		return true;
	}

	FreeLink();

	return false;
}

void CEavooStatisticHelper::ShouldStop(void)
{
	mShouldStop = true;
}

bool CEavooStatisticHelper::ShowProject(const char *projectname)
{
	if (projectname == NULL)
	{
		return false;
	}

	return ShowProject(FindProject(projectname));
}

bool CEavooStatisticHelper::ShowProject(CMonthSellLink *head)
{
	int total = 0;
	char buff[32];

	m_list_sell.DeleteAllItems();

	for (CMonthSellNode *p = head->mHead; p; p = p->next)
	{
		sprintf(buff, "%04d年%02d月", p->mYear, p->mMonth);
		int index = m_list_sell.InsertItem(m_list_sell.GetItemCount(), buff);
		sprintf(buff, "%d 台", p->mSellCount);
		m_list_sell.SetItemText(index, 1, buff);
		total += p->mSellCount;
	}

	sprintf(buff, "总销量：%d 台", total);
	m_static_status.SetWindowText(buff);

	return true;
}

bool CEavooStatisticHelper::ShowProject(void)
{
	TCITEM item;
	char projectname[16] = {0};
	item.pszText = projectname;
	item.cchTextMax = sizeof(projectname);
	item.mask = TCIF_TEXT;
	if (m_tab_sell.GetItem(m_tab_sell.GetCurSel(), &item) == false)
	{
		return false;
	}

	ShowProject(projectname);

	return true;
}