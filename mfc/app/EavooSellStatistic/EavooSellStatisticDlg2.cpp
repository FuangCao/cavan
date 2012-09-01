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
: CDialog(CEavooSellStatisticDlg2::IDD, pParent), mHelper(m_progress_statistic, m_static_status)
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

bool CEavooSellStatisticDlg2::ShowCurrentProject(void)
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

	mHelper.ShowProject(projectname, m_list_sell);
	return true;
}

int CEavooSellStatisticDlg2::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg2 *dlg = (CEavooSellStatisticDlg2 *)data;
	dlg->m_button_ok.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	dlg->m_button_export.EnableWindow(false);
	if (dlg->mHelper.EavooSellStatistic(theApp.mDatabasePath, dlg->m_tab_sell))
	{
		dlg->ShowCurrentProject();
	}
	else
	{
		AfxMessageBox("无法进行统计，可能数据库已经损坏");
	}
	dlg->mThread = NULL;
	dlg->m_button_ok.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);
	dlg->m_button_export.EnableWindow(true);

	return 0;
}

void CEavooSellStatisticDlg2::OnSelchangeTABsell(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	ShowCurrentProject();
}

void CEavooSellStatisticDlg2::OnBUTTONstop() 
{
	// TODO: Add your control notification handler code here
	mHelper.ShouldStop();
}

int CEavooSellStatisticDlg2::ThreadHandlerExport(void *data)
{
	CEavooSellStatisticDlg2 *dlg = (CEavooSellStatisticDlg2 *)data;
	CFileDialog fileDlg(false, "txt", "eavoo_statistic", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, "(*.txt)|*.txt|(*.xml)|*.xml||");
	if (fileDlg.DoModal() != IDOK)
	{
		return -1;
	}

	CFile file;
	if (file.Open(fileDlg.GetPathName(), CFile::modeWrite | CFile::modeCreate | CFile::shareDenyNone, NULL) == false)
	{
		AfxMessageBox("打开文件失败");
		return -1;
	}

	dlg->m_button_ok.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	bool result;
	CString fileExt = fileDlg.GetFileExt();
	if (fileExt.CompareNoCase("txt") == 0)
	{
		result = dlg->mHelper.ExportTxtFile(file);
	}
	else if (fileExt.CompareNoCase("xml") == 0)
	{
		result = dlg->mHelper.ExportXmlFile(file);
	}
	else
	{
		AfxMessageBox("不支持的类型");
		result = false;
	}

	file.Close();

	if (result)
	{
		AfxMessageBox("导出数据完成");
	}
	else
	{
		AfxMessageBox("导出数据失败");
	}

	dlg->m_button_ok.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);

	return 0;
}

void CEavooSellStatisticDlg2::OnBUTTONexport() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread((AFX_THREADPROC)ThreadHandlerExport, this);
}
