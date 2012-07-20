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

CMonthSellNode::CMonthSellNode(const char *month)
{
	strcpy(mMonth, month);
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

CMonthSellNode *CMonthSellLink::FindMonth(const char *month)
{
	for (CMonthSellNode *p = mHead; p; p = p->next)
	{
		if (strcmp(month, p->mMonth) == 0)
		{
			return p;
		}
	}

	return NULL;
}

bool CMonthSellLink::AddMonthSellCount(const char *month, int count)
{
	CMonthSellNode *p = FindMonth(month);
	if (p == NULL)
	{
		p = new CMonthSellNode(month);
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

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg2 message handlers

BOOL CEavooSellStatisticDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_list_sell.InsertColumn(0, "月份", LVCFMT_LEFT, 180);
	m_list_sell.InsertColumn(1, "销量", LVCFMT_LEFT, 160);

	EavooSellParseFile(CACHE_FILENAME);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

char *CEavooSellStatisticDlg2::AdbParseDataMulti(const char *buff, const char *end)
{
	char mobile[32];
	char time[32];
	char content[1024];
	char *segments[] =
	{
		mobile, time, content
	};

	while (1)
	{
		const char *temp = CEavooSellStatisticDlg::AdbParseDataSingle(buff, end, segments, NELEM(segments));
		if (temp == NULL)
		{
			break;
		}

		time[7] = 0;
		mLink.AddMonthSellCount(time, 1);

		for (buff = temp; buff < end && (*buff == '\r' || *buff == '\n'); buff++);
	}

	return (char *)buff;
}

char *CEavooSellStatisticDlg2::AdbParseDataMain(char *buff, char *end)
{
	char *p = AdbParseDataMulti(buff, end);

	while (p < end)
	{
		*buff++ = *p++;
	}

	return buff;
}

bool CEavooSellStatisticDlg2::EavooSellParseFile(const char *filename)
{
	CFile file;

	if (file.Open(filename, CFile::modeRead | CFile::shareDenyRead) == false)
	{
		return false;
	}

	char buff[4096], *p, *p_end;
	int readLen;

	p = buff;
	p_end = buff + sizeof(buff);
	mLink.InitLinkHead();

	while (1)
	{
		readLen = file.Read(p, p_end - p);
		if (readLen == 0)
		{
			break;
		}

		p = AdbParseDataMain(buff, p + readLen);
	}

	file.Close();

	m_list_sell.DeleteAllItems();
	int total = 0;

	for (CMonthSellNode *pNode = mLink.mHead; pNode; pNode = pNode->next)
	{
		m_list_sell.InsertItem(0, pNode->mMonth);
		sprintf(buff, "%d台", pNode->mSellCount);
		m_list_sell.SetItemText(0, 1, buff);
		total += pNode->mSellCount;
	}

	sprintf(buff, "%d台", total);
	m_static_total_sell.SetWindowText(buff);

	mLink.InitLinkHead();

	return true;
}
