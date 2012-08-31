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
}

CEavooSellStatisticDlg2::~CEavooSellStatisticDlg2()
{
	mHelper.FreeLink();
}

void CEavooSellStatisticDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEavooSellStatisticDlg2)
	DDX_Control(pDX, IDC_BUTTON_export, m_button_export);
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
	ON_BN_CLICKED(IDC_BUTTON_export, OnBUTTONexport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

int CEavooSellStatisticDlg2::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg2 *dlg = (CEavooSellStatisticDlg2 *)data;
	dlg->m_button_ok.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	dlg->mHelper.EavooSellStatistic(theApp.mDatabasePath);
	dlg->mThread = NULL;
	dlg->m_button_ok.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);

	return 0;
}

void CEavooSellStatisticDlg2::OnSelchangeTABsell(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here

	mHelper.ShowProject();
	*pResult = 0;
}

void CEavooSellStatisticDlg2::OnBUTTONstop() 
{
	// TODO: Add your control notification handler code here
	mHelper.ShouldStop();
}

void CEavooSellStatisticDlg2::OnBUTTONexport() 
{
	// TODO: Add your control notification handler code here
	
}
