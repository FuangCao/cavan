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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg message handlers

bool CEavooSellStatisticDlg::Initialize(void)
{
	char ip[32];
	m_ipaddress1.GetWindowText(ip, sizeof(ip));

	return mMessage.Initialize(DEFAULT_CACHE_FILENAME, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone, m_edit_port, ip);
}

void CEavooSellStatisticDlg::Uninitialize(void)
{
	mMessage.Uninitialize();
}

// EDUA# + �ֻ���IMEI�� + ��,�� + �ֻ�����汾�� + ��,,,,#��

int CEavooSellStatisticDlg::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;
	CEavooShortMessage &message = dlg->mMessage;

	if (dlg->Initialize() == false)
	{
		dlg->mThread = NULL;
		return -1;
	}

	AfxMessageBox("������������");
	dlg->m_button_start.EnableWindow(false);
	dlg->m_button_stop.EnableWindow(true);
	dlg->m_button_load.EnableWindow(false);
	dlg->m_button_clean_database.EnableWindow(false);

	while (dlg->mThread)
	{
		dlg->ShowStatus("���ӵ�ADB ...");

		while (message.AdbLocalConnect() == false && dlg->mThread)
		{
			Sleep(2000);
		}

		if (dlg->mThread == NULL)
		{
			break;
		}

		dlg->ShowStatus("���ӵ��ֻ� ...");

		if (message.AdbServerConnect() == false)
		{
			Sleep(2000);

			for (int i = 5; i > 0 && dlg->mThread; i--)
			{
				dlg->ShowStatus("����ʧ�ܣ�%d �����������", i);
				Sleep(1000);
			}

			continue;
		}

		dlg->ShowStatus("�������� ...");

		while (message.ReceiveFromNetwork())
		{
			message.InsertIntoList(dlg->m_list_sms);
		}
	}

	dlg->Uninitialize();
	dlg->mThread = NULL;
	AfxMessageBox("��������ֹͣ����");
	dlg->ShowStatus("��������ֹͣ����");
	dlg->m_button_clean_database.EnableWindow(true);
	dlg->m_button_load.EnableWindow(true);
	dlg->m_button_start.EnableWindow(true);
	dlg->m_button_stop.EnableWindow(false);

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
	m_list_sms.InsertColumn(0, "�ֻ�����", LVCFMT_LEFT, 130);
	m_list_sms.InsertColumn(1, "����ʱ��", LVCFMT_LEFT, 200);
	m_list_sms.InsertColumn(2, "���ŵ�����", LVCFMT_LEFT, 320);

	m_ipaddress1.SetWindowText(DEFAULT_SERVER_IP);
	m_static_state.SetWindowText("ֹͣ");

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
		mThread = AfxBeginThread((AFX_THREADPROC)CEavooSellStatisticDlg::ThreadHandler, this);
	}
}

void CEavooSellStatisticDlg::OnBUTTONstop()
{
	// TODO: Add your control notification handler code here
	if (mThread)
	{
		mThread = NULL;
	}

	Uninitialize();
}

void CEavooSellStatisticDlg::OnBUTTONclean()
{
	// TODO: Add your control notification handler code here
	m_list_sms.DeleteAllItems();
}

void CEavooSellStatisticDlg::OnBUTTONload()
{
	// TODO: Add your control notification handler code here
	if (mThread != 0)
	{
		AfxMessageBox("����ֹͣ������");
		return;
	}

	m_list_sms.DeleteAllItems();

	CEavooShortMessage message;
	if (message.Initialize(DEFAULT_CACHE_FILENAME, CFile::modeRead | CFile::shareDenyNone) == false)
	{
		return;
	}

	while (message.ReadFromFile())
	{
		message.InsertIntoList(m_list_sms);
	}

	message.Uninitialize();
}

void CEavooSellStatisticDlg::OnBUTTONcleandatabase()
{
	// TODO: Add your control notification handler code here
	if (AfxMessageBox("����ɾ�����޷��ָ������Ҫɾ����", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CFile file;
		file.Open(DEFAULT_CACHE_FILENAME, CFile::modeWrite | CFile::shareDenyNone, NULL);
		file.SetLength(0);
		file.Close();

		AfxMessageBox("���ݿ������");
		m_list_sms.DeleteAllItems();
	}
}
