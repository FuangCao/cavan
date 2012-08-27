// EavooSellStatisticDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooSellStatisticDlg.h"
#include "EavooSellStatisticDlg2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg2 dialog


CEavooSellStatisticDlg2::CEavooSellStatisticDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(CEavooSellStatisticDlg2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEavooSellStatisticDlg2)
	//}}AFX_DATA_INIT
	mHead = NULL;
}

CEavooSellStatisticDlg2::~CEavooSellStatisticDlg2()
{
	FreeLink();
}

void CEavooSellStatisticDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEavooSellStatisticDlg2)
	DDX_Control(pDX, IDC_BUTTON_stop, m_button_stop);
	DDX_Control(pDX, IDOK, m_button_ok);
	DDX_Control(pDX, IDC_STATIC_status, m_static_status);
	DDX_Control(pDX, IDC_PROGRESS_statistic, m_progress_statistic);
	DDX_Control(pDX, IDC_TAB_sell, m_tab_sell);
	DDX_Control(pDX, IDC_LIST_sell, m_list_sell);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEavooSellStatisticDlg2, CDialog)
	//{{AFX_MSG_MAP(CEavooSellStatisticDlg2)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_sell, OnSelchangeTABsell)
	ON_BN_CLICKED(IDC_BUTTON_stop, OnBUTTONstop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
// CEavooSellStatisticDlg2 message handlers

void CEavooSellStatisticDlg2::FreeLink(void)
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

CMonthSellLink *CEavooSellStatisticDlg2::FindProject(const char *projectname)
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

bool CEavooSellStatisticDlg2::EavooSellStatisticBase(const char *pathname)
{
	FreeLink();

	CEavooShortMessageHelper helper;
	if (helper.Initialize(pathname, CFile::modeRead | CFile::shareDenyNone) == false)
	{
		return false;
	}

	struct tm *time;
	char projectname[16];
	CEavooShortMessageBody body;
	CMonthSellLink *project;
	DWORD dwTotalLength = helper.GetFileLength();
	double dbReadLength = 0;

	m_progress_statistic.SetRange32(0, 100);
	mShouldStop = false;

	while (1)
	{
		if (mShouldStop)
		{
			AfxMessageBox("取消统计");
			m_static_status.SetWindowText("取消统计");
			return false;
		}

		m_progress_statistic.SetPos((int) (dbReadLength / dwTotalLength * 100));

		DWORD length = helper.ReadFromFile();
		if (length <= 0)
		{
			break;
		}

		dbReadLength += length;

		m_static_status.SetWindowText(helper.GetMessageBody());
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

	return true;
}

bool CEavooSellStatisticDlg2::EavooSellStatistic(const char *pathname)
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

bool CEavooSellStatisticDlg2::ShowProject(const char *projectname)
{
	if (projectname == NULL)
	{
		return false;
	}

	return ShowProject(FindProject(projectname));
}

bool CEavooSellStatisticDlg2::ShowProject(CMonthSellLink *head)
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

bool CEavooSellStatisticDlg2::ShowProject(void)
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

int CEavooSellStatisticDlg2::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg2 *dlg = (CEavooSellStatisticDlg2 *)data;
	dlg->m_button_ok.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	dlg->EavooSellStatistic(DEFAULT_CACHE_FILENAME);
	dlg->mThread = NULL;
	dlg->m_button_ok.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);

	return 0;
}

BOOL CEavooSellStatisticDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_list_sell.InsertColumn(0, "月份", LVCFMT_LEFT, 100);
	m_list_sell.InsertColumn(1, "销量", LVCFMT_LEFT, 200);

	mThread = AfxBeginThread((AFX_THREADPROC)ThreadHandler, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEavooSellStatisticDlg2::OnSelchangeTABsell(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here

	ShowProject();
	*pResult = 0;
}

void CEavooSellStatisticDlg2::OnBUTTONstop() 
{
	// TODO: Add your control notification handler code here
	mShouldStop = true;
}
