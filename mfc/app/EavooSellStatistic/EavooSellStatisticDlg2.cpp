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
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CEavooSellStatisticDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEavooSellStatisticDlg2)
	DDX_Control(pDX, IDC_LIST_sell, m_list_sell);
	DDX_Control(pDX, IDC_STATIC_total_sell, m_static_total_sell);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEavooSellStatisticDlg2, CDialog)
	//{{AFX_MSG_MAP(CEavooSellStatisticDlg2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonthSell methods

CMonthSellNode::CMonthSellNode(int year, int month)
{
	mYear = year;
	mMonth = month;
	mSellCount = 0;
}

CMonthSellLink::~CMonthSellLink(void)
{
	InitLinkHead();
}

void CMonthSellLink::InitLinkHead(void)
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
		p = new CMonthSellNode(year, month);
		if (p == NULL)
		{
			return false;
		}

		p->next = mHead;
		mHead = p;
	}

	p->mSellCount += count;

	return true;
}

bool CMonthSellLink::EavooSellStatistic(const char *pathname)
{
	CFile file;

	if (file.Open(pathname, CFile::modeRead | CFile::shareDenyNone, NULL) == false)
	{
		return false;
	}

	CEavooShortMessage message(file, 0, NULL);
	if (message.Initialize() < 0)
	{
		return false;
	}

	InitLinkHead();

	struct tm *time;

	while (1)
	{
		if (message.ReadFromFile() < 0)
		{
			break;
		}

		time = gmtime(message.GetDate());
		if (time == NULL)
		{
			continue;
		}

		if (AddMonthSellCount(time->tm_year + 1900, time->tm_mon, 1) == false)
		{
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg2 message handlers

BOOL CEavooSellStatisticDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_list_sell.InsertColumn(0, "�·�", LVCFMT_LEFT, 180);
	m_list_sell.InsertColumn(1, "����", LVCFMT_LEFT, 160);

	CMonthSellLink link;
	if (link.EavooSellStatistic(CACHE_FILENAME) == false)
	{
		return FALSE;
	}

	int total = 0;
	char buff[32];
	for (CMonthSellNode *p = link.mHead; p; p = p->next)
	{
		sprintf(buff, "%04d��%02d��", p->mYear, p->mMonth);
		m_list_sell.InsertItem(0, buff);
		sprintf(buff, "%d(̨)", p->mSellCount);
		m_list_sell.SetItemText(0, 1, buff);
		total += p->mSellCount;
	}

	sprintf(buff, "%d(̨)", total);
	m_static_total_sell.SetWindowText(buff);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}