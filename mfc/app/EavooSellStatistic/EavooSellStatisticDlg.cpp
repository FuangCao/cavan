// EavooSellStatisticDlg.cpp : implementation file
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
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg dialog

CEavooSellStatisticDlg::CEavooSellStatisticDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEavooSellStatisticDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEavooSellStatisticDlg)
	m_edit_port = 8888;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mThread = NULL;
}

void CEavooSellStatisticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEavooSellStatisticDlg)
	DDX_Control(pDX, IDC_BUTTON_statistic, m_button_statistic);
	DDX_Control(pDX, IDC_BUTTON_clean, m_button_clean);
	DDX_Control(pDX, IDC_BUTTON_export, m_button_export);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_BUTTON_import, m_button_import);
	DDX_Control(pDX, IDC_BUTTON_clean_database, m_button_clean_database);
	DDX_Control(pDX, IDC_BUTTON_load, m_button_load);
	DDX_Control(pDX, IDC_BUTTON_stop, m_button_stop);
	DDX_Control(pDX, IDC_BUTTON_start, m_button_start);
	DDX_Control(pDX, IDC_STATIC_state, m_static_state);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipaddress1);
	DDX_Control(pDX, IDC_LIST_sms, m_list_sms);
	DDX_Text(pDX, IDC_EDIT_port, m_edit_port);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEavooSellStatisticDlg, CDialog)
	//{{AFX_MSG_MAP(CEavooSellStatisticDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_statistic, OnBUTTONstatistic)
	ON_BN_CLICKED(IDC_BUTTON_stop, OnBUTTONstop)
	ON_BN_CLICKED(IDC_BUTTON_start, OnBUTTONstart)
	ON_BN_CLICKED(IDC_BUTTON_clean, OnBUTTONclean)
	ON_BN_CLICKED(IDC_BUTTON_load, OnBUTTONload)
	ON_BN_CLICKED(IDC_BUTTON_clean_database, OnBUTTONcleandatabase)
	ON_BN_CLICKED(IDC_BUTTON_export, OnBUTTONexport)
	ON_BN_CLICKED(IDC_BUTTON_import, OnBUTTONimport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg message handlers

// EDUA# + 手机的IMEI号 + “,” + 手机软件版本号 + “,,,,#”

int CEavooSellStatisticDlg::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;
	CEavooShortMessageHelper helper;
	char ip[32];
	dlg->m_ipaddress1.GetWindowText(ip, sizeof(ip));

	if (helper.Initialize(CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone, NULL, dlg->m_edit_port, ip) == false)
	{
		dlg->mThread = NULL;
		return -1;
	}

	dlg->m_button_start.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	dlg->m_button_load.EnableWindow(false);
	dlg->m_button_clean_database.EnableWindow(false);
	dlg->m_button_import.EnableWindow(false);
	dlg->m_button_export.EnableWindow(false);

	while (dlg->mThread)
	{
		dlg->ShowStatus("连接到ADB ...");

		while (helper.AdbLocalConnect() == false && dlg->mThread)
		{
			Sleep(2000);
		}

		if (dlg->mThread == NULL)
		{
			break;
		}

		dlg->ShowStatus("连接到手机 ...");

		if (helper.AdbServerConnect() == false)
		{
			Sleep(2000);

			for (int i = 5; i > 0 && dlg->mThread; i--)
			{
				dlg->ShowStatus("连接失败，%d 秒后重新连接", i);
				Sleep(1000);
			}

			continue;
		}

		dlg->ShowStatus("正在运行 ...");

		while (helper.ReceiveFromNetwork() && helper.WriteToFile())
		{
			helper.GetShortMessage().InsertIntoList(dlg->m_list_sms);
		}
	}

	helper.Uninitialize();
	dlg->mThread = NULL;
	AfxMessageBox("服务器已停止工作");
	dlg->ShowStatus("服务器已停止工作");
	dlg->m_button_clean_database.EnableWindow(true);
	dlg->m_button_load.EnableWindow(true);
	dlg->m_button_start.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);
	dlg->m_button_import.EnableWindow(true);
	dlg->m_button_export.EnableWindow(true);

	return 0;
}

void CEavooSellStatisticDlg::ShowStatus(const char *format, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, format);
	vsprintf(buff, format, ap);
	va_end(ap);

	m_static_state.SetWindowText(buff);
}

BOOL CEavooSellStatisticDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_list_sms.InsertColumn(0, "手机号码", LVCFMT_LEFT, 130);
	m_list_sms.InsertColumn(1, "发送时间", LVCFMT_LEFT, 160);
	m_list_sms.InsertColumn(2, "短信的内容", LVCFMT_LEFT, 320);

	m_ipaddress1.SetWindowText(DEFAULT_SERVER_IP);
	m_static_state.SetWindowText("停止");

	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEavooSellStatisticDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEavooSellStatisticDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEavooSellStatisticDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEavooSellStatisticDlg::OnBUTTONstatistic()
{
	CEavooSellStatisticDlg2 Dlg;

	Dlg.DoModal();
}

void CEavooSellStatisticDlg::OnBUTTONstart()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (mThread == NULL)
	{
		mThread = AfxBeginThread((AFX_THREADPROC)ThreadHandler, this);
	}
}

void CEavooSellStatisticDlg::OnBUTTONstop()
{
	// TODO: Add your control notification handler code here
	if (mThread)
	{
		mThread = NULL;
	}
}

void CEavooSellStatisticDlg::OnBUTTONclean()
{
	// TODO: Add your control notification handler code here
	m_list_sms.DeleteAllItems();
}

int CEavooSellStatisticDlg::ThreadHandlerLoad(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;

	dlg->m_list_sms.DeleteAllItems();

	CEavooShortMessageHelper helper;
	if (helper.Initialize(CFile::modeRead | CFile::shareDenyNone) == false)
	{
		return -1;
	}

	dlg->m_button_import.EnableWindow(false);
	dlg->m_button_export.EnableWindow(false);
	dlg->m_button_start.EnableWindow(false);
	dlg->m_button_clean_database.EnableWindow(false);
	dlg->m_button_clean.EnableWindow(false);
	dlg->m_button_load.EnableWindow(false);

	DWORD totalLength, rdLength;
	double rdTotal, percent;
	unsigned char count;

	totalLength = helper.GetFileLength();
	rdTotal = 0;
	dlg->m_progress.SetRange(0, 100);

	char buff[8];

	for (count = 0; ; count++)
	{
		rdLength = helper.ReadFromFile();
		if ((count & PROGRESS_MIN_COUNT) == 0 || rdLength == 0)
		{
			percent = rdTotal * 100 / totalLength;
			dlg->m_progress.SetPos((int)percent);
			sprintf(buff, "%0.2lf%%", percent);
			dlg->m_static_state.SetWindowText(buff);

			if (rdLength == 0)
			{
				break;
			}
		}

		helper.GetShortMessage().InsertIntoList(dlg->m_list_sms);

		rdTotal += rdLength;
	}

	helper.Uninitialize();

	dlg->m_button_import.EnableWindow(true);
	dlg->m_button_export.EnableWindow(true);
	dlg->m_button_start.EnableWindow(true);
	dlg->m_button_clean_database.EnableWindow(true);
	dlg->m_button_clean.EnableWindow(true);
	dlg->m_button_load.EnableWindow(true);

	return 0;
}

void CEavooSellStatisticDlg::OnBUTTONload()
{
	// TODO: Add your control notification handler code here
	if (mThread != 0)
	{
		AfxMessageBox("请先停止服务器");
		return;
	}

	AfxBeginThread((AFX_THREADPROC)ThreadHandlerLoad, this);
}

void CEavooSellStatisticDlg::OnBUTTONcleandatabase()
{
	// TODO: Add your control notification handler code here
	if (AfxMessageBox("数据删除后将无法恢复，真的要删除吗？", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CFile file;
		file.Open(theApp.mDatabasePath, CFile::modeWrite, NULL);
		file.SetLength(0);
		file.Close();

		AfxMessageBox("数据库已清空");
		m_list_sms.DeleteAllItems();
	}
}

int CEavooSellStatisticDlg::ThreadHandlerExport(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;

	CFileDialog fileDlg(false, "txt", "eavoo_sell", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, "(*.txt)|*.txt|(*.xml)|*.xml||");
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

	CEavooShortMessageHelper helper;
	if (helper.Initialize(CFile::modeRead | CFile::shareDenyNone) == false)
	{
		AfxMessageBox("打开数据库失败");
		file.Close();
		return -1;
	}

	bool result;

	dlg->m_button_import.EnableWindow(false);
	dlg->m_button_export.EnableWindow(false);
	dlg->m_button_start.EnableWindow(false);
	dlg->m_button_clean_database.EnableWindow(false);

	CString fileExt = fileDlg.GetFileExt();
	if (fileExt.CompareNoCase("txt") == 0)
	{
		result = helper.ExportTextFile(file, dlg->m_progress, dlg->m_static_state);
	}
	else if (fileExt.CompareNoCase("xml") == 0)
	{
		result = helper.ExportXmlFile(file, dlg->m_progress, dlg->m_static_state);
	}
	else
	{
		AfxMessageBox("不支持的类型");
		result = false;
	}

	file.Close();
	helper.Uninitialize();

	if (result)
	{
		AfxMessageBox("导出数据完成");
	}
	else
	{
		AfxMessageBox("导出数据失败");
	}

	dlg->m_button_import.EnableWindow(true);
	dlg->m_button_export.EnableWindow(true);
	dlg->m_button_start.EnableWindow(true);
	dlg->m_button_clean_database.EnableWindow(true);

	return 0;
}

int CEavooSellStatisticDlg::ThreadHandlerImport(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;
	CEavooShortMessageHelper helperRead, helperWrite;

	CFileDialog fileDlg(true, "dat", "eavoo_sell", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "(*.dat)|*.dat||");
	if (fileDlg.DoModal() != IDOK)
	{
		return -1;
	}

	if (helperRead.Initialize(CFile::modeRead, fileDlg.GetPathName()) == false)
	{
		return -1;
	}

	if (helperWrite.Initialize(CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate) == false)
	{
		return -1;
	}

	dlg->m_button_import.EnableWindow(false);
	dlg->m_button_export.EnableWindow(false);
	dlg->m_button_start.EnableWindow(false);
	dlg->m_button_load.EnableWindow(false);
	dlg->m_button_clean_database.EnableWindow(false);
	dlg->m_button_statistic.EnableWindow(false);

	if (helperWrite.ImportDatabase(helperRead, dlg->m_progress, dlg->m_static_state))
	{
		AfxMessageBox("导入数据库成功");
	}
	else
	{
		AfxMessageBox("导入数据库失败");
	}

	dlg->m_button_import.EnableWindow(true);
	dlg->m_button_export.EnableWindow(true);
	dlg->m_button_start.EnableWindow(true);
	dlg->m_button_load.EnableWindow(true);
	dlg->m_button_clean_database.EnableWindow(true);
	dlg->m_button_statistic.EnableWindow(true);

	return 0;
}

void CEavooSellStatisticDlg::OnBUTTONexport()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread((AFX_THREADPROC)ThreadHandlerExport, this);
}

void CEavooSellStatisticDlg::OnBUTTONimport()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread((AFX_THREADPROC)ThreadHandlerImport, this);
}
